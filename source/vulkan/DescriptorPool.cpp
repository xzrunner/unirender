#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/TypeConverter.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

DescriptorPool::DescriptorPool(const std::shared_ptr<LogicalDevice>& device, size_t max_sets, 
                               const std::vector<std::pair<DescriptorType, size_t>>& pool_sizes)
	: m_device(device)
{
    std::vector<VkDescriptorPoolSize> vk_pool_sizes(pool_sizes.size());
    for (int i = 0, n = pool_sizes.size(); i < n; ++i) 
    {
        auto& src = pool_sizes[i];
        auto& dst = vk_pool_sizes[i];
        dst.type = TypeConverter::To(src.first);
        dst.descriptorCount = src.second;
    }

    VkDescriptorPoolCreateInfo desc_pool_ci = {};
    desc_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    desc_pool_ci.pNext = NULL;
    desc_pool_ci.maxSets = max_sets;
    desc_pool_ci.poolSizeCount = vk_pool_sizes.size();
    desc_pool_ci.pPoolSizes = vk_pool_sizes.data();

    if (vkCreateDescriptorPool(m_device->GetHandler(), &desc_pool_ci, NULL, &m_handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(m_device->GetHandler(), m_handle, nullptr);
}

}
}