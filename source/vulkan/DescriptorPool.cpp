#include "unirender/vulkan/DescriptorPool.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

DescriptorPool::DescriptorPool(VkDevice device)
	: m_device(device)
{
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(m_device, m_handle, nullptr);
}

void DescriptorPool::Create(bool use_texture)
{
    /* DEPENDS on init_uniform_buffer() and
     * init_descriptor_and_pipeline_layouts() */

    VkResult res;
    VkDescriptorPoolSize type_count[2];
    type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    type_count[0].descriptorCount = 1;
    if (use_texture) {
        type_count[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        type_count[1].descriptorCount = 1;
    }

    VkDescriptorPoolCreateInfo descriptor_pool = {};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = NULL;
    descriptor_pool.maxSets = 1;
    descriptor_pool.poolSizeCount = use_texture ? 2 : 1;
    descriptor_pool.pPoolSizes = type_count;

    res = vkCreateDescriptorPool(m_device, &descriptor_pool, NULL, &m_handle);
    assert(res == VK_SUCCESS);
}

}
}