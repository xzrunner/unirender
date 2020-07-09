#include "unirender/vulkan/CommandPool.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

CommandPool::CommandPool(VkDevice device)
    : m_device(device)
{

}

CommandPool::~CommandPool()
{
    vkDestroyCommandPool(m_device, m_handle, NULL);
}

void CommandPool::Create()
{
    VkResult res;

    VkCommandPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.pNext = NULL;
    cmd_pool_info.queueFamilyIndex = m_queue_family_index;
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    res = vkCreateCommandPool(m_device, &cmd_pool_info, NULL, &m_handle);
    assert(res == VK_SUCCESS);
}

}
}