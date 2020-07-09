#include "unirender/vulkan/PipelineCache.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

PipelineCache::PipelineCache(VkDevice device)
    : m_device(device)
{
}

PipelineCache::~PipelineCache()
{
    vkDestroyPipelineCache(m_device, m_handle, NULL);
}

void PipelineCache::Create()
{
    VkPipelineCacheCreateInfo pipelineCache;
    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCache.pNext = NULL;
    pipelineCache.initialDataSize = 0;
    pipelineCache.pInitialData = NULL;
    pipelineCache.flags = 0;
    VkResult res = vkCreatePipelineCache(m_device, &pipelineCache, NULL, &m_handle);
    assert(res == VK_SUCCESS);
}

}
}