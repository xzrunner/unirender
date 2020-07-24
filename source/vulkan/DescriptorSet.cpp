#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

DescriptorSet::DescriptorSet(const std::shared_ptr<LogicalDevice>& device, const DescriptorPool& pool,
                             const std::vector<std::shared_ptr<DescriptorSetLayout>>& _layouts,
                             const std::vector<VkWriteDescriptorSet>& descriptors)
	: m_device(device)
{
    std::vector<VkDescriptorSetLayout> layouts(_layouts.size());
    for (size_t i = 0, n = _layouts.size(); i < n; ++i) {
        layouts[i] = _layouts[i]->GetHandler();
    }

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext = NULL;
    alloc_info[0].descriptorPool = pool.GetHandler();
    alloc_info[0].descriptorSetCount = layouts.size();
    alloc_info[0].pSetLayouts = layouts.data();

    VkResult res = vkAllocateDescriptorSets(m_device->GetHandler(), alloc_info, &m_handle);
    assert(res == VK_SUCCESS);

    for (auto& desc : descriptors) {
        const_cast<VkWriteDescriptorSet&>(desc).dstSet = m_handle;
    }
    vkUpdateDescriptorSets(m_device->GetHandler(), descriptors.size(), descriptors.data(), 0, NULL);
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::AddDescriptor(std::vector<VkWriteDescriptorSet>& descriptors,
                                  VkDescriptorType type, const VkDescriptorBufferInfo* buffer_info)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = type;
    writeDescriptorSet.dstBinding = descriptors.size();
    writeDescriptorSet.pBufferInfo = buffer_info;
    writeDescriptorSet.descriptorCount = 1;
    descriptors.push_back(writeDescriptorSet);
}

void DescriptorSet::AddDescriptor(std::vector<VkWriteDescriptorSet>& descriptors,
                                  VkDescriptorType type, const VkDescriptorImageInfo* image_info)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = type;
    writeDescriptorSet.dstBinding = descriptors.size();
    writeDescriptorSet.pImageInfo = image_info;
    writeDescriptorSet.descriptorCount = 1;
    descriptors.push_back(writeDescriptorSet);
}

}
}