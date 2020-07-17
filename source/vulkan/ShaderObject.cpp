#include "unirender/vulkan/ShaderObject.h"
#include "unirender/vulkan/TypeConverter.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

ShaderObject::ShaderObject(VkDevice dev, ShaderType type, const std::vector<unsigned int>& spirv)
    : m_dev(dev)
{
    Init(type, spirv);
}

ShaderObject::~ShaderObject()
{
    vkDestroyShaderModule(m_dev, m_stage.module, NULL);
}

void ShaderObject::Init(ShaderType type, const std::vector<unsigned int>& spirv)
{
    VkShaderModuleCreateInfo shader_ci{};
    shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_ci.pNext = nullptr;
    shader_ci.flags = 0;
    shader_ci.codeSize = spirv.size() * sizeof(uint32_t);
    shader_ci.pCode = spirv.data();

    m_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stage.pNext = NULL;
    m_stage.pSpecializationInfo = NULL;
    m_stage.flags = 0;
    m_stage.stage = TypeConverter::To(type);
    m_stage.pName = "main";
    VkResult res = vkCreateShaderModule(m_dev, &shader_ci, NULL, &m_stage.module);
    assert(res == VK_SUCCESS);
}

}
}