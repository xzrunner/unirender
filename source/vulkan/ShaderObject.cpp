#include "unirender/vulkan/ShaderObject.h"
#include "unirender/vulkan/TypeConverter.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

ShaderObject::ShaderObject(const std::shared_ptr<LogicalDevice>& device, 
                           ShaderType type, const std::vector<unsigned int>& spirv)
    : m_device(device)
{
    VkShaderModuleCreateInfo shader_ci{};
    shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_ci.pNext = nullptr;
    shader_ci.flags = 0;
    shader_ci.codeSize = spirv.size() * sizeof(uint32_t);
    shader_ci.pCode = spirv.data();

    m_handle.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_handle.pNext = NULL;
    m_handle.pSpecializationInfo = NULL;
    m_handle.flags = 0;
    m_handle.stage = TypeConverter::To(type);
    m_handle.pName = "main";
    VkResult res = vkCreateShaderModule(m_device->GetHandler(), &shader_ci, NULL, &m_handle.module);
    assert(res == VK_SUCCESS);
}

ShaderObject::~ShaderObject()
{
    vkDestroyShaderModule(m_device->GetHandler(), m_handle.module, NULL);
}

}
}