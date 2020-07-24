#include "unirender/vulkan/PipelineCache.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

PipelineCache::PipelineCache(const std::shared_ptr<LogicalDevice>& device)
    : m_device(device)
{
    VkPipelineCacheCreateInfo pipelineCache;
    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCache.pNext = NULL;
    pipelineCache.initialDataSize = 0;
    pipelineCache.pInitialData = NULL;
    pipelineCache.flags = 0;
    VkResult res = vkCreatePipelineCache(m_device->GetHandler(), &pipelineCache, NULL, &m_handle);
    assert(res == VK_SUCCESS);
}

PipelineCache::~PipelineCache()
{
    vkDestroyPipelineCache(m_device->GetHandler(), m_handle, NULL);
}

}
}