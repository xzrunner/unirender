#include "unirender/vulkan/CommandBuffer.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

CommandBuffer::CommandBuffer(const std::shared_ptr<LogicalDevice>& device,
                             const std::shared_ptr<CommandPool>& pool)
    : m_device(device)
    , m_pool(pool)
{
    VkCommandBufferAllocateInfo cmd_info = {};
    cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_info.pNext = NULL;
    cmd_info.commandPool = m_pool->GetHandler();
    cmd_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_info.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device->GetHandler(), &cmd_info, &m_handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command buffer!");
    }
}

CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(m_device->GetHandler(), m_pool->GetHandler(), 1, &m_handle);
}

VkCommandBuffer CommandBuffer::BeginSingleTimeCommands(VkDevice device, VkCommandPool cmd_pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmd_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cb;
    vkAllocateCommandBuffers(device, &allocInfo, &cb);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cb, &beginInfo);

    return cb;
}

void CommandBuffer::EndSingleTimeCommands(VkCommandBuffer cb, VkDevice device, 
                                          VkCommandPool cmd_pool, VkQueue graphics_queue)
{
    vkEndCommandBuffer(cb);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cb;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, cmd_pool, 1, &cb);
}

}
}