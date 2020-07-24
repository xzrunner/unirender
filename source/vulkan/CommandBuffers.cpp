#include "unirender/vulkan/CommandBuffers.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

CommandBuffers::CommandBuffers(const std::shared_ptr<LogicalDevice>& device,
                               const std::shared_ptr<CommandPool>& pool, int count)
    : m_device(device)
    , m_pool(pool)
{
    m_handle.resize(count);

    VkCommandBufferAllocateInfo cmd = {};
    cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd.pNext = NULL;
    cmd.commandPool = m_pool->GetHandler();
    cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd.commandBufferCount = count;

    VkResult res = vkAllocateCommandBuffers(m_device->GetHandler(), &cmd, m_handle.data());
    assert(res == VK_SUCCESS);
}

CommandBuffers::~CommandBuffers()
{
    vkFreeCommandBuffers(m_device->GetHandler(), m_pool->GetHandler(), static_cast<uint32_t>(m_handle.size()), m_handle.data());
}

}
}