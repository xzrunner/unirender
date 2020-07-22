#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/UniformBuffer.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

DescriptorSet::DescriptorSet(VkDevice device)
	: m_device(device)
{
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::Create(const DescriptorPool& pool)
{
    std::vector<VkDescriptorSetLayout> layouts(m_layouts.size());
    for (size_t i = 0, n = m_layouts.size(); i < n; ++i) {
        layouts[i] = m_layouts[i]->GetHandler();
    }

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext = NULL;
    alloc_info[0].descriptorPool = pool.GetHandler();
    alloc_info[0].descriptorSetCount = layouts.size();
    alloc_info[0].pSetLayouts = layouts.data();

    VkResult res = vkAllocateDescriptorSets(m_device, alloc_info, &m_handle);
    assert(res == VK_SUCCESS);

    for (auto& desc : m_descriptors) {
        desc.dstSet = m_handle;
    }
    vkUpdateDescriptorSets(m_device, m_descriptors.size(), m_descriptors.data(), 0, NULL);
}

void DescriptorSet::AddDescriptor(VkDescriptorType type, const VkDescriptorBufferInfo* buffer_info)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = type;
    writeDescriptorSet.dstBinding = m_descriptors.size();
    writeDescriptorSet.pBufferInfo = buffer_info;
    writeDescriptorSet.descriptorCount = 1;
    m_descriptors.push_back(writeDescriptorSet);
}

void DescriptorSet::AddDescriptor(VkDescriptorType type, const VkDescriptorImageInfo* image_info)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = type;
    writeDescriptorSet.dstBinding = m_descriptors.size();
    writeDescriptorSet.pImageInfo = image_info;
    writeDescriptorSet.descriptorCount = 1;
    m_descriptors.push_back(writeDescriptorSet);
}

}
}