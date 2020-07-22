#include "unirender/vulkan/DescriptorSetLayout.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

DescriptorSetLayout::DescriptorSetLayout(VkDevice device)
	: m_device(device)
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_device, m_handle, NULL);
}

void DescriptorSetLayout::Create()
{
    VkDescriptorSetLayoutCreateInfo desc_set_layout_ci;
    desc_set_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc_set_layout_ci.pNext = NULL;
    desc_set_layout_ci.flags = 0;
    desc_set_layout_ci.bindingCount = m_bindings.size();
    desc_set_layout_ci.pBindings = m_bindings.data();

    VkResult res = vkCreateDescriptorSetLayout(m_device, &desc_set_layout_ci, NULL, &m_handle);
    assert(res == VK_SUCCESS);
}

void DescriptorSetLayout::AddBinding(VkDescriptorType desc_type, VkShaderStageFlags stage_flags)
{
    VkDescriptorSetLayoutBinding binding;
    binding.binding = m_bindings.size();
    binding.descriptorType = desc_type;
    binding.descriptorCount = 1;
    binding.stageFlags = stage_flags;
    binding.pImmutableSamplers = NULL;
    m_bindings.push_back(binding);
}

}
}