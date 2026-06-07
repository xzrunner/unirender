#pragma once

#include "unirender/ShaderProgram.h"
#include "unirender/ShaderType.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace ur
{

class Uniform;

namespace vulkan
{

class ShaderObject;
class LogicalDevice;
class PhysicalDevice;
class UniformBuffer;

// One entry in the reflection-derived descriptor set layout. After binding
// reassignment every resource has a unique binding within set 0. For UBOs `ubo`
// holds the host-visible backing buffer the named uniforms write into; for
// image/sampler descriptors `ubo` is null and the Context binds the engine's
// currently-bound texture/sampler at draw time.
struct DescBinding
{
    uint32_t            binding = 0;
    VkDescriptorType    type    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VkShaderStageFlags  stages  = 0;
    std::shared_ptr<UniformBuffer> ubo; // non-null only for UNIFORM_BUFFER
};

class ShaderProgram : public ur::ShaderProgram
{
public:
    ShaderProgram(const std::shared_ptr<LogicalDevice>& device,
        const PhysicalDevice& phy_dev,
        const std::vector<unsigned int>& vs, const std::vector<unsigned int>& fs);
    ShaderProgram(const std::vector<unsigned int>& cs);
    virtual ~ShaderProgram();

    virtual void Bind() const override;
    virtual bool CheckStatus() const override;

    virtual void GetComputeWorkGroupSize(int& x, int& y, int& z) const override;

    virtual int QueryTexSlot(const std::string& name) const override;
    virtual int QueryAttrLoc(const std::string& name) const override;
    virtual int QueryImgSlot(const std::string& name) const override;

    virtual void BindSSBO(const std::string& name, int idx,
        const std::shared_ptr<StorageBuffer>& ssbo) const override {}

    virtual bool HasStage(ShaderType stage) const override;

    auto& GetShaderStages() const { return m_shader_stages; }

    // Reflection-derived descriptor layout + matching pipeline layout. The Vulkan
    // Context builds its descriptor set and pipeline from these so the bound
    // resources always match the shader's actual (deconflicted) bindings.
    const std::vector<DescBinding>& GetDescBindings() const { return m_desc_bindings; }
    VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_desc_layout; }
    VkPipelineLayout      GetVkPipelineLayout()      const { return m_pipe_layout; }

private:
    // Reflect one stage, reassign each resource a unique set-0 binding (starting
    // at *next_binding), patch the SPIR-V binding decorations in-place, create the
    // per-block UBOs + named uniforms, and append to m_desc_bindings.
    void ReflectStage(const std::shared_ptr<LogicalDevice>& device, const PhysicalDevice& phy_dev,
        std::vector<unsigned int>& spirv, ShaderType stage, uint32_t& next_binding);

    void BuildLayouts(const std::shared_ptr<LogicalDevice>& device);

private:
    std::shared_ptr<LogicalDevice> m_device = nullptr;

    std::shared_ptr<ShaderObject> m_vs = nullptr;
    std::shared_ptr<ShaderObject> m_fs = nullptr;

    VkPipelineShaderStageCreateInfo m_shader_stages[2];

    std::vector<DescBinding> m_desc_bindings;
    std::unordered_map<std::string, int> m_tex_slots; // name -> reassigned binding

    VkDescriptorSetLayout m_desc_layout = VK_NULL_HANDLE;
    VkPipelineLayout      m_pipe_layout = VK_NULL_HANDLE;

    bool m_has_vs = false;
    bool m_has_fs = false;

}; // ShaderProgram

}
}
