#import <Metal/Metal.h>
#include "unirender/metal/ShaderProgram.h"

// Optional: SPIRV-Cross for SPIR-V -> MSL translation
// #include <spirv_cross/spirv_msl.hpp>

#include <cassert>
#include <iostream>

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
    if (m_frag_func)   { CFRelease(m_frag_func);   m_frag_func = nullptr; }
    if (m_vert_func)   { CFRelease(m_vert_func);   m_vert_func = nullptr; }
    if (m_mtl_library) { CFRelease(m_mtl_library);  m_mtl_library = nullptr; }
}

void ShaderProgram::CompileFromSPIRV(const std::vector<unsigned int>& vs,
                                     const std::vector<unsigned int>& fs)
{
    // SPIRV-Cross translation: SPIR-V -> MSL
    // This requires linking spirv-cross-msl. A minimal example:
    //
    // spirv_cross::CompilerMSL vs_compiler(vs);
    // spirv_cross::CompilerMSL fs_compiler(fs);
    // auto vs_msl = vs_compiler.compile();
    // auto fs_msl = fs_compiler.compile();
    //
    // For now, combine both into a single library source.
    // In production you'd compile each separately or use argument buffers.

    // Stub: mark as invalid when SPIRV-Cross is not linked
    std::cerr << "[Metal] SPIR-V -> MSL cross-compilation requires spirv-cross-msl. "
              << "Use the MSL constructor or link SPIRV-Cross.\n";
    m_valid = false;
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
    return -1; // Metal uses explicit buffer/texture indices in MSL [[texture(N)]]
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
