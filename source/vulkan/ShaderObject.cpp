#include "unirender/vulkan/ShaderObject.h"
#include "unirender/vulkan/TypeConverter.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

ShaderObject::ShaderObject(VkDevice dev, ShaderType type, const std::string& source)
    : m_dev(dev)
{
    VkShaderModuleCreateInfo shader_ci{};
    shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_ci.codeSize = source.size();
    shader_ci.pCode = (uint32_t*)source.c_str();

    m_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stage.pNext = NULL;
    m_stage.pSpecializationInfo = NULL;
    m_stage.flags = 0;
    m_stage.stage = TypeConverter::To(type);
    m_stage.pName = "main";
    VkResult res = vkCreateShaderModule(dev, &shader_ci, NULL, &m_stage.module);
    assert(res == VK_SUCCESS);
}

ShaderObject::~ShaderObject()
{
    vkDestroyShaderModule(m_dev, m_stage.module, NULL);
}

}
}