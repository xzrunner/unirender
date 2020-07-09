#include "unirender/vulkan/CommandBuffers.h"
#include "unirender/vulkan/CommandPool.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

CommandBuffers::CommandBuffers(VkDevice device, const std::shared_ptr<CommandPool>& pool)
    : m_device(device)
    , m_pool(pool)
{
}

CommandBuffers::~CommandBuffers()
{
    vkFreeCommandBuffers(m_device, m_pool->GetHandle(), static_cast<uint32_t>(m_handle.size()), m_handle.data());
}

void CommandBuffers::Create(int count)
{
    m_handle.resize(count);

    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = m_pool->GetHandle();
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = count;

    VkResult res = vkAllocateCommandBuffers(m_device, &cmd, m_handle.data());
    assert(res == VK_SUCCESS);
}

}
}