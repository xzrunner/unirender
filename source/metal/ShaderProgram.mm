#import <Metal/Metal.h>
#include "unirender/metal/ShaderProgram.h"

#include <spirv_msl.hpp> // SPIRV-Cross: SPIR-V -> MSL cross-compilation

#include "unirender/Uniform.h"

#include <iostream>
#include <stdexcept>
#include <cstring>

namespace
{
// A uniform that writes directly into a uniform block's backing MTLBuffer
// (shared storage) at a fixed byte offset. SetValue's n is the scalar count.
class MtlUniform : public ur::Uniform
{
public:
    MtlUniform(const std::string& name, uint8_t* dst)
        : ur::Uniform(name, ur::UniformType::Float1, 1, 0), m_dst(dst) {}
    void Clean() override {}
    void SetValue(const int* v, int n) override
        { if (m_dst && v) { std::memcpy(m_dst, v, (size_t)n * sizeof(int)); } }
    void SetValue(const float* v, int n) override
        { if (m_dst && v) { std::memcpy(m_dst, v, (size_t)n * sizeof(float)); } }
private:
    uint8_t* m_dst = nullptr;
};
} // anonymous namespace

namespace ur
{
namespace metal
{

ShaderProgram::ShaderProgram(void* mtl_device,
                             const std::vector<unsigned int>& vs,
                             const std::vector<unsigned int>& fs)
    : m_mtl_device(mtl_device)
{
    CompileFromSPIRV(vs, fs);
}

ShaderProgram::ShaderProgram(void* mtl_device, const std::string& msl_source)
    : m_mtl_device(mtl_device)
{
    CompileFromMSL(msl_source);
}

ShaderProgram::~ShaderProgram()
{
    if (m_frag_func)      { CFRelease(m_frag_func);      m_frag_func = nullptr; }
    if (m_vert_func)      { CFRelease(m_vert_func);      m_vert_func = nullptr; }
    if (m_vert_func_flip) { CFRelease(m_vert_func_flip); m_vert_func_flip = nullptr; }
    if (m_mtl_library) { CFRelease(m_mtl_library);  m_mtl_library = nullptr; }
    for (auto& u : m_ubos) { if (u.mtl_buffer) { CFRelease(u.mtl_buffer); } }
    m_ubos.clear();
}

void ShaderProgram::CompileFromSPIRV(const std::vector<unsigned int>& vs,
                                     const std::vector<unsigned int>& fs)
{
    if (vs.empty() || fs.empty()) { m_valid = false; return; }

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_mtl_device;

    // Translate one SPIR-V stage to MSL (renaming its entry point to a stable
    // name) AND reflect its uniform blocks + textures, so QueryUniform()/
    // QueryTexSlot() resolve and Draw() knows which MSL buffer/texture index
    // each resource binds to. Captures `this` to populate member state.
    auto compile_stage = [this](const std::vector<unsigned int>& spirv,
                                spv::ExecutionModel model, ShaderType stage,
                                const char* entry_name, id<MTLDevice> dev) -> std::string
    {
        spirv_cross::CompilerMSL comp(spirv);
        spirv_cross::CompilerMSL::Options opts; // .platform defaults to macOS
        opts.set_msl_version(2, 0);
        // Metal requires a fragment color output to have at least as many
        // components as its attachment's pixel format. Several MRT shaders write
        // vec3 (e.g. the deferred GBuffer's normal target), and Metal has no
        // 3-component 16F format -- it is promoted to RGBA16Float (4 comp), so a
        // vec3 output makes pipeline creation fail. Pad every fragment output to
        // 4 components so any attachment format is satisfied. GL keeps the vec3
        // source; this only affects the MSL translation.
        opts.pad_fragment_output_components = true;
        comp.set_msl_options(opts);
        for (auto& ep : comp.get_entry_points_and_stages()) {
            if (ep.execution_model == model) {
                comp.rename_entry_point(ep.name, entry_name, model);
                break;
            }
        }
        std::string msl = comp.compile();

        // Resource MSL indices are only valid after compile().
        spirv_cross::ShaderResources res = comp.get_shader_resources();
        for (auto& ubo : res.uniform_buffers) {
            unsigned int idx = comp.get_automatic_msl_resource_binding(ubo.id);
            const spirv_cross::SPIRType& type = comp.get_type(ubo.base_type_id);
            size_t size = comp.get_declared_struct_size(type);
            if (size == 0) { continue; }
            // get_declared_struct_size() returns the std140 size up to the end of
            // the last member WITHOUT the trailing round-up (e.g. { vec3; vec3;
            // vec3; } -> 44). MSL pads the block up to its 16-byte alignment (48),
            // and Metal validates the bound buffer against that MSL size -- a 44B
            // buffer trips "argument has a length(48)" and asserts. Round up to 16
            // to match the MSL block size (get_declared_struct_size_msl() would be
            // exact but is a protected CompilerMSL member). Member offsets coincide
            // between std140 and MSL for scalar/vec/mat members, so the per-uniform
            // write offsets below stay correct.
            size = (size + 15u) & ~size_t(15u);
            // SPIRV-Cross returns ~0u for a block it optimized out (e.g. all members
            // unused). Binding a buffer at that index in Metal is out of range and
            // crashes setVertex/FragmentBuffer -- skip it.
            if (idx == (unsigned int)-1 || idx >= 31u) { continue; }
            id<MTLBuffer> buf = [dev newBufferWithLength:size
                                                 options:MTLResourceStorageModeShared];
            uint8_t* contents = (uint8_t*)[buf contents];
            std::memset(contents, 0, size);
            m_ubos.push_back({ stage, idx, size, (__bridge_retained void*)buf });

            const std::string block = comp.get_name(ubo.id); // instance name, e.g. "u_fs"
            unsigned int n = (unsigned int)type.member_types.size();
            for (unsigned int i = 0; i < n; ++i) {
                std::string mname = comp.get_member_name(ubo.base_type_id, i);
                size_t off = comp.type_struct_member_offset(type, i);
                std::string full = block.empty() ? mname : (block + "." + mname);
                AddUniform(full, std::make_shared<MtlUniform>(full, contents + off));
            }
        }
        auto valid_idx = [](unsigned int v) { return v != (unsigned int)-1 && v < 31u; };
        for (auto& img : res.sampled_images) {
            unsigned int idx = comp.get_automatic_msl_resource_binding(img.id);
            if (valid_idx(idx)) { m_tex_slots[comp.get_name(img.id)] = (int)idx; }
        }
        // HLSL Texture2D + SamplerState (via DXC) become SEPARATE SPIR-V image
        // and sampler objects, not combined sampled_images -- reflect those too,
        // else m_tex_slots stays empty and the texture never binds at its MSL
        // index (-> sampler reads 0 -> black).
        for (auto& img : res.separate_images) {
            unsigned int idx = comp.get_automatic_msl_resource_binding(img.id);
            if (valid_idx(idx)) { m_tex_slots[comp.get_name(img.id)] = (int)idx; }
        }
        for (auto& smp : res.separate_samplers) {
            unsigned int idx = comp.get_automatic_msl_resource_binding(smp.id);
            if (valid_idx(idx)) { m_sampler_slots[comp.get_name(smp.id)] = (int)idx; }
        }
        return msl;
    };

    // Each stage becomes its own MTLLibrary: combining two independent MSL
    // translation units into one source collides on helper/struct definitions.
    std::string vs_msl, fs_msl;
    try {
        vs_msl = compile_stage(vs, spv::ExecutionModelVertex,   ShaderType::VertexShader,   "vertex_main",   device);
        fs_msl = compile_stage(fs, spv::ExecutionModelFragment, ShaderType::FragmentShader, "fragment_main", device);
    } catch (const std::exception& e) {
        std::cerr << "[Metal] SPIRV-Cross SPIR-V -> MSL failed: " << e.what() << "\n";
        m_valid = false; return;
    }

    // Reserve a vertex (stage_in) buffer index above any vertex-stage uniform
    // block so the two never collide on the same MSL buffer slot.
    m_vtx_buf_index = 0;
    for (auto& u : m_ubos) {
        if (u.stage == ShaderType::VertexShader && (int)u.buffer_index >= m_vtx_buf_index) {
            m_vtx_buf_index = (int)u.buffer_index + 1;
        }
    }

    NSError* err = nil;

    id<MTLLibrary> vlib = [device newLibraryWithSource:[NSString stringWithUTF8String:vs_msl.c_str()]
                                               options:nil error:&err];
    if (!vlib) {
        std::cerr << "[Metal] vertex MSL compile failed: "
                  << (err ? [[err localizedDescription] UTF8String] : "(unknown)") << "\n";
        m_valid = false; return;
    }
    id<MTLFunction> vfunc = [vlib newFunctionWithName:@"vertex_main"];

    // A second vertex function with clip-space y negated, for offscreen (FBO)
    // passes -- Metal render-to-texture is top-left origin while OpenGL (which the
    // engine's UV math assumes) is bottom-left, so without this every FBO render
    // is vertically mirrored. SPIRV-Cross emits the entry's [[position]] output as
    // `out.gl_Position` and returns `out;`, so inject the negation before it.
    {
        std::string vs_flip = vs_msl;
        size_t rp = vs_flip.rfind("return out;");
        if (rp != std::string::npos && vs_flip.find("out.gl_Position") != std::string::npos) {
            vs_flip.insert(rp, "out.gl_Position.y = -out.gl_Position.y;\n    ");
            NSError* ferr = nil;
            id<MTLLibrary> vlib_flip =
                [device newLibraryWithSource:[NSString stringWithUTF8String:vs_flip.c_str()]
                                     options:nil error:&ferr];
            id<MTLFunction> vfunc_flip = vlib_flip ? [vlib_flip newFunctionWithName:@"vertex_main"] : nil;
            if (vfunc_flip) { m_vert_func_flip = (__bridge_retained void*)vfunc_flip; }
        }
    }

    err = nil;
    id<MTLLibrary> flib = [device newLibraryWithSource:[NSString stringWithUTF8String:fs_msl.c_str()]
                                               options:nil error:&err];
    if (!flib) {
        std::cerr << "[Metal] fragment MSL compile failed: "
                  << (err ? [[err localizedDescription] UTF8String] : "(unknown)") << "\n";
        m_valid = false; return;
    }
    id<MTLFunction> ffunc = [flib newFunctionWithName:@"fragment_main"];

    if (vfunc) { m_vert_func = (__bridge_retained void*)vfunc; }
    if (ffunc) { m_frag_func = (__bridge_retained void*)ffunc; }
    // Keep the vertex library for GetMTLLibrary(); the fragment library stays
    // alive through ffunc (an MTLFunction retains its library).
    m_mtl_library = (__bridge_retained void*)vlib;

    m_valid = (m_vert_func != nullptr && m_frag_func != nullptr);
    if (!m_valid) {
        std::cerr << "[Metal] MSL entry point not found (vertex_main / fragment_main)\n";
    }
}

void ShaderProgram::CompileFromMSL(const std::string& source)
{
    if (source.empty()) {
        m_valid = false;
        return;
    }

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_mtl_device;

    NSError* error = nil;
    NSString* src = [NSString stringWithUTF8String:source.c_str()];
    MTLCompileOptions* opts = [[MTLCompileOptions alloc] init];

    id<MTLLibrary> library = [device newLibraryWithSource:src options:opts error:&error];
    if (!library) {
        std::cerr << "[Metal] Shader compilation failed: "
                  << [[error localizedDescription] UTF8String] << "\n";
        m_valid = false;
        return;
    }
    m_mtl_library = (__bridge_retained void*)library;

    // Look for standard entry points
    id<MTLFunction> vertFunc = [library newFunctionWithName:@"vertex_main"];
    if (!vertFunc) {
        vertFunc = [library newFunctionWithName:@"vertexShader"];
    }
    if (vertFunc) {
        m_vert_func = (__bridge_retained void*)vertFunc;
    }

    id<MTLFunction> fragFunc = [library newFunctionWithName:@"fragment_main"];
    if (!fragFunc) {
        fragFunc = [library newFunctionWithName:@"fragmentShader"];
    }
    if (fragFunc) {
        m_frag_func = (__bridge_retained void*)fragFunc;
    }

    m_valid = (m_vert_func != nullptr && m_frag_func != nullptr);
}

void ShaderProgram::GetComputeWorkGroupSize(int& x, int& y, int& z) const
{
    x = y = z = 1; // TODO: query from compute pipeline
}

int ShaderProgram::QueryTexSlot(const std::string& name) const
{
    auto it = m_tex_slots.find(name);
    return it != m_tex_slots.end() ? it->second : -1;
}

int ShaderProgram::QueryAttrLoc(const std::string& name) const
{
    return -1; // Metal uses explicit attribute indices [[attribute(N)]]
}

int ShaderProgram::QueryImgSlot(const std::string& name) const
{
    return -1;
}

bool ShaderProgram::HasStage(ShaderType stage) const
{
    switch (stage)
    {
    case ShaderType::VertexShader:   return m_vert_func != nullptr;
    case ShaderType::FragmentShader: return m_frag_func != nullptr;
    default:                         return false;
    }
}

}
}
