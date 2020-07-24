#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

DescriptorSetLayout::DescriptorSetLayout(const std::shared_ptr<LogicalDevice>& device,
                                         const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	: m_device(device)
{
    VkDescriptorSetLayoutCreateInfo desc_set_layout_ci;
    desc_set_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc_set_layout_ci.pNext = NULL;
    desc_set_layout_ci.flags = 0;
    desc_set_layout_ci.bindingCount = bindings.size();
    desc_set_layout_ci.pBindings = bindings.data();

    VkResult res = vkCreateDescriptorSetLayout(device->GetHandler(), &desc_set_layout_ci, NULL, &m_handle);
    assert(res == VK_SUCCESS);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_device->GetHandler(), m_handle, NULL);
}

void DescriptorSetLayout::AddBinding(std::vector<VkDescriptorSetLayoutBinding>& bindings,
                                     VkDescriptorType desc_type, VkShaderStageFlags stage_flags)
{
    VkDescriptorSetLayoutBinding binding;
    binding.binding = bindings.size();
    binding.descriptorType = desc_type;
    binding.descriptorCount = 1;
    binding.stageFlags = stage_flags;
    binding.pImmutableSamplers = NULL;
    bindings.push_back(binding);
}

}
}