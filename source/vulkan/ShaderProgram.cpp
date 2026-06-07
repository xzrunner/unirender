#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/ShaderObject.h"
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/ShaderType.h"
#include "unirender/Uniform.h"

#include <spirv_cross.hpp> // SPIR-V reflection (no cross-compile)

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <assert.h>

namespace
{

// A uniform that writes n scalars into its block's CPU staging blob at a fixed
// byte offset, then uploads the whole block into the host-visible VkBuffer. All
// members of one block share the same blob + UniformBuffer. n is the SCALAR count
// (mat4 => 16, vec3 => 3, float => 1), matching the engine's SetValue convention.
class VulkanUniform : public ur::Uniform
{
public:
    VulkanUniform(const std::string& name,
                  const std::shared_ptr<std::vector<uint8_t>>& blob,
                  const std::shared_ptr<ur::vulkan::UniformBuffer>& ubo,
                  size_t offset)
        : ur::Uniform(name, ur::UniformType::Float1, 1, 0)
        , m_blob(blob), m_ubo(ubo), m_offset(offset) {}

    void Clean() override {}
    void SetValue(const int* v, int n) override   { Write(v, (size_t)n * sizeof(int)); }
    void SetValue(const float* v, int n) override { Write(v, (size_t)n * sizeof(float)); }

private:
    void Write(const void* v, size_t bytes)
    {
        if (!m_blob || !v || bytes == 0) { return; }
        auto& blob = *m_blob;
        if (m_offset >= blob.size()) { return; }
        if (m_offset + bytes > blob.size()) { bytes = blob.size() - m_offset; }
        std::memcpy(blob.data() + m_offset, v, bytes);
        if (m_ubo) { m_ubo->Update(blob.data(), blob.size()); }
        if (std::getenv("TT_VK_UNIF") && bytes >= 4) {
            float f0; std::memcpy(&f0, v, 4);
            std::cerr << "[vkunif] " << GetName() << " off=" << m_offset
                      << " bytes=" << bytes << " v0=" << f0 << std::endl;
        }
    }

    std::shared_ptr<std::vector<uint8_t>>      m_blob;
    std::shared_ptr<ur::vulkan::UniformBuffer> m_ubo;
    size_t                                     m_offset = 0;
};

// Overwrite the literal of an OpDecorate <id> <decoration> <value> instruction.
// SPIR-V: header is 5 words; each instruction word0 = (wordCount<<16)|opcode.
// OpDecorate = 71; Binding decoration = 33, DescriptorSet = 34.
void PatchDecoration(std::vector<unsigned int>& spirv, uint32_t id,
                     uint32_t decoration, uint32_t value)
{
    size_t i = 5;
    while (i < spirv.size())
    {
        uint32_t word = spirv[i];
        uint16_t wc = (uint16_t)(word >> 16);
        uint16_t op = (uint16_t)(word & 0xFFFFu);
        if (wc == 0) { break; }
        if (op == 71u && i + 3 < spirv.size() &&
            spirv[i + 1] == id && spirv[i + 2] == decoration) {
            spirv[i + 3] = value;
        }
        i += wc;
    }
}

} // anonymous namespace

namespace ur
{
namespace vulkan
{

ShaderProgram::ShaderProgram(const std::shared_ptr<LogicalDevice>& device,
                             const PhysicalDevice& phy_dev,
                             const std::vector<unsigned int>& vs,
                             const std::vector<unsigned int>& fs)
    : m_device(device)
{
    // Work on copies: ReflectStage reassigns each resource a unique set-0 binding
    // and patches the binding decorations in-place (DXC gives every stage's first
    // cbuffer binding 0, and t#/s# share a binding -- both illegal once combined
    // into one pipeline). The patched SPIR-V is what the shader modules are built
    // from, so the modules and the descriptor layout always agree.
    std::vector<unsigned int> vs2 = vs;
    std::vector<unsigned int> fs2 = fs;

    uint32_t next_binding = 0;
    try {
        ReflectStage(device, phy_dev, vs2, ShaderType::VertexShader,   next_binding);
        ReflectStage(device, phy_dev, fs2, ShaderType::FragmentShader, next_binding);
    } catch (const std::exception& e) {
        std::cerr << "[Vulkan] SPIR-V reflection failed: " << e.what() << std::endl;
    }

    m_vs = std::make_shared<ShaderObject>(device, ShaderType::VertexShader,   vs2);
    m_fs = std::make_shared<ShaderObject>(device, ShaderType::FragmentShader, fs2);
    m_shader_stages[0] = m_vs->GetHandler();
    m_shader_stages[1] = m_fs->GetHandler();

    m_has_vs = !vs.empty();
    m_has_fs = !fs.empty();

    BuildLayouts(device);
}

ShaderProgram::ShaderProgram(const std::vector<unsigned int>& cs)
{
}

ShaderProgram::~ShaderProgram()
{
    if (m_device) {
        auto dev = m_device->GetHandler();
        if (m_pipe_layout) { vkDestroyPipelineLayout(dev, m_pipe_layout, nullptr); }
        if (m_desc_layout) { vkDestroyDescriptorSetLayout(dev, m_desc_layout, nullptr); }
    }
}

void ShaderProgram::ReflectStage(const std::shared_ptr<LogicalDevice>& device,
                                 const PhysicalDevice& phy_dev,
                                 std::vector<unsigned int>& spirv, ShaderType stage,
                                 uint32_t& next_binding)
{
    if (spirv.empty()) { return; }

    const VkShaderStageFlags stage_bit = (stage == ShaderType::VertexShader)
        ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
    const bool dump = std::getenv("TT_VK_REFLECT") != nullptr;
    const char* sname = (stage == ShaderType::VertexShader) ? "VS" : "FS";

    spirv_cross::Compiler comp(spirv.data(), spirv.size());
    spirv_cross::ShaderResources res = comp.get_shader_resources();

    auto assign = [&](uint32_t id) -> uint32_t {
        uint32_t b = next_binding++;
        PatchDecoration(spirv, id, 33u /*Binding*/, b);
        PatchDecoration(spirv, id, 34u /*DescriptorSet*/, 0u);
        return b;
    };

    // Uniform blocks -> host-visible UBO + named uniforms per member.
    for (auto& ubo : res.uniform_buffers)
    {
        const spirv_cross::SPIRType& type = comp.get_type(ubo.base_type_id);
        size_t size = comp.get_declared_struct_size(type);
        if (size == 0) { continue; }
        size = (size + 15u) & ~size_t(15u);

        uint32_t binding = assign(ubo.id);

        auto blob = std::make_shared<std::vector<uint8_t>>(size, 0);
        auto ubuf = std::make_shared<UniformBuffer>(device, phy_dev, blob->data(), size);

        DescBinding db;
        db.binding = binding; db.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        db.stages = stage_bit; db.ubo = ubuf;
        m_desc_bindings.push_back(db);

        const std::string block = comp.get_name(ubo.id);
        unsigned int n = (unsigned int)type.member_types.size();
        for (unsigned int i = 0; i < n; ++i) {
            std::string mname = comp.get_member_name(ubo.base_type_id, i);
            size_t off = comp.type_struct_member_offset(type, i);
            std::string full = block.empty() ? mname : (block + "." + mname);
            AddUniform(full, std::make_shared<VulkanUniform>(full, blob, ubuf, off));
        }
        if (dump) std::cerr << "[VKbind " << sname << "] UBO '" << block << "' -> binding " << binding << std::endl;
    }

    auto add_img = [&](const spirv_cross::Resource& r, VkDescriptorType t, const char* kind) {
        uint32_t binding = assign(r.id);
        DescBinding db; db.binding = binding; db.type = t; db.stages = stage_bit;
        m_desc_bindings.push_back(db);
        m_tex_slots[comp.get_name(r.id)] = (int)binding;
        if (dump) std::cerr << "[VKbind " << sname << "] " << kind << " '" << comp.get_name(r.id)
                            << "' -> binding " << binding << std::endl;
    };
    for (auto& img : res.sampled_images)    { add_img(img, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, "combined"); }
    for (auto& img : res.separate_images)   { add_img(img, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, "image"); }
    for (auto& smp : res.separate_samplers) { add_img(smp, VK_DESCRIPTOR_TYPE_SAMPLER, "sampler"); }
}

void ShaderProgram::BuildLayouts(const std::shared_ptr<LogicalDevice>& device)
{
    auto dev = device->GetHandler();

    std::vector<VkDescriptorSetLayoutBinding> binds;
    binds.reserve(m_desc_bindings.size());
    for (auto& b : m_desc_bindings) {
        VkDescriptorSetLayoutBinding lb{};
        lb.binding         = b.binding;
        lb.descriptorType  = b.type;
        lb.descriptorCount = 1;
        lb.stageFlags      = b.stages;
        binds.push_back(lb);
    }

    VkDescriptorSetLayoutCreateInfo ci{};
    ci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount = (uint32_t)binds.size();
    ci.pBindings    = binds.empty() ? nullptr : binds.data();
    if (vkCreateDescriptorSetLayout(dev, &ci, nullptr, &m_desc_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create reflected descriptor set layout!");
    }

    VkPipelineLayoutCreateInfo pci{};
    pci.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pci.setLayoutCount = 1;
    pci.pSetLayouts    = &m_desc_layout;
    if (vkCreatePipelineLayout(dev, &pci, nullptr, &m_pipe_layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create reflected pipeline layout!");
    }
}

void ShaderProgram::Bind() const
{
}

bool ShaderProgram::CheckStatus() const
{
	return true;
}

void ShaderProgram::GetComputeWorkGroupSize(int& x, int& y, int& z) const
{
    x = y = z = 1;
}

int ShaderProgram::QueryTexSlot(const std::string& name) const
{
    auto it = m_tex_slots.find(name);
    return it != m_tex_slots.end() ? it->second : -1;
}

int ShaderProgram::QueryAttrLoc(const std::string& name) const
{
    return -1;
}

int ShaderProgram::QueryImgSlot(const std::string& name) const
{
    return -1;
}

bool ShaderProgram::HasStage(ShaderType stage) const
{
    switch (stage)
    {
    case ShaderType::VertexShader:   return m_has_vs;
    case ShaderType::FragmentShader: return m_has_fs;
    default:                         return false;
    }
}

}
}
