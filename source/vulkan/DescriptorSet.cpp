#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/ContextInfo.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/DeviceInfo.h"
#include "unirender/vulkan/UniformBuffer.h"

#include <assert.h>

#define NUM_DESCRIPTOR_SETS 1

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

void DescriptorSet::Create(const DeviceInfo& dev_info, const ContextInfo& ctx_info, bool use_texture)
{
    /* DEPENDS on init_descriptor_pool() */

    VkResult res;

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext = NULL;
    alloc_info[0].descriptorPool = ctx_info.desc_pool->GetHandler();
    alloc_info[0].descriptorSetCount = NUM_DESCRIPTOR_SETS;
    alloc_info[0].pSetLayouts = ctx_info.desc_set_layout->GetHandler().data();

    m_handle.resize(NUM_DESCRIPTOR_SETS);
    res = vkAllocateDescriptorSets(m_device, alloc_info, m_handle.data());
    assert(res == VK_SUCCESS);

    VkWriteDescriptorSet writes[2];

    writes[0] = {};
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].pNext = NULL;
    writes[0].dstSet = m_handle[0];
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &ctx_info.uniform_buf->GetBufferInfo();
    writes[0].dstArrayElement = 0;
    writes[0].dstBinding = 0;

    if (use_texture)
	{
        writes[1] = {};
        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = m_handle[0];
        writes[1].dstBinding = 1;
        writes[1].descriptorCount = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[1].pImageInfo = &dev_info.texture_data.image_info;
        writes[1].dstArrayElement = 0;
    }

    vkUpdateDescriptorSets(m_device, use_texture ? 2 : 1, writes, 0, NULL);
}

}
}