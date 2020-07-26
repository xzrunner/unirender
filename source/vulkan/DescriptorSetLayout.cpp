#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/TypeConverter.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

DescriptorSetLayout::DescriptorSetLayout(const std::shared_ptr<LogicalDevice>& device,
                                         const std::vector<std::pair<DescriptorType, ShaderType>>& bindings)
	: m_device(device)
{
    std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
    vk_bindings.reserve(bindings.size());
    for (auto& itr : bindings) 
    {
        VkDescriptorSetLayoutBinding binding;
        binding.binding = vk_bindings.size();
        binding.descriptorType = TypeConverter::To(itr.first);
        binding.descriptorCount = 1;
        binding.stageFlags = TypeConverter::To(itr.second);
        binding.pImmutableSamplers = NULL;
        vk_bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo desc_set_layout_ci;
    desc_set_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc_set_layout_ci.pNext = NULL;
    desc_set_layout_ci.flags = 0;
    desc_set_layout_ci.bindingCount = vk_bindings.size();
    desc_set_layout_ci.pBindings = vk_bindings.data();

    VkResult res = vkCreateDescriptorSetLayout(device->GetHandler(), &desc_set_layout_ci, NULL, &m_handle);
    assert(res == VK_SUCCESS);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(m_device->GetHandler(), m_handle, NULL);
}

}
}