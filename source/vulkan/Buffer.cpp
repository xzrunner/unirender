#include "unirender/vulkan/Buffer.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Utility.h"
#include "unirender/vulkan/CommandBuffer.h"

#include <stdexcept>
#include <cstring>

namespace ur
{
namespace vulkan
{

Buffer::Buffer(const std::shared_ptr<LogicalDevice>& device)
    : m_device(device)
{
}

Buffer::~Buffer()
{
    Clear();
}

void Buffer::Create(VkPhysicalDevice phy_dev, VkDeviceSize size,
                    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    // FIX: destroy previous buffer/memory before re-creating
    Clear();

    m_size = size;

    VkBufferCreateInfo buf_ci{};
    buf_ci.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_ci.size        = size;
    buf_ci.usage       = usage;
    buf_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto vk_dev = m_device->GetHandler();
    if (vkCreateBuffer(vk_dev, &buf_ci, nullptr, &m_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(vk_dev, m_buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize  = mem_reqs.size;
    alloc_info.memoryTypeIndex = Utility::FindMemoryType(
        phy_dev, mem_reqs.memoryTypeBits, properties);

    if (vkAllocateMemory(vk_dev, &alloc_info, nullptr, &m_memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    if (vkBindBufferMemory(vk_dev, m_buffer, m_memory, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to bind buffer memory!");
    }
}

void Buffer::Upload(const void* data, size_t size)
{
    if (!data || size == 0) return;

    void* buf;
    auto vk_dev = m_device->GetHandler();
    if (vkMapMemory(vk_dev, m_memory, 0, size, 0, &buf) != VK_SUCCESS) {
        throw std::runtime_error("failed to map buffer memory!");
    }
    memcpy(buf, data, size);
    vkUnmapMemory(vk_dev, m_memory);
}

void Buffer::CopyFrom(const Buffer& src, size_t size,
                       VkCommandPool cmd_pool, VkQueue graphics_queue)
{
    m_size = size;

    VkCommandBuffer cb = CommandBuffer::BeginSingleTimeCommands(
        m_device->GetHandler(), cmd_pool);

    VkBufferCopy copy_region{};
    copy_region.size = size;
    vkCmdCopyBuffer(cb, src.GetHandler(), m_buffer, 1, &copy_region);

    CommandBuffer::EndSingleTimeCommands(
        cb, m_device->GetHandler(), cmd_pool, graphics_queue);
}

void Buffer::Clear()
{
    if (!m_device) return;

    auto vk_dev = m_device->GetHandler();
    if (vk_dev == VK_NULL_HANDLE) return;

    // Defer destruction instead of freeing immediately: this buffer may still be
    // referenced by a draw already recorded into the current frame's command buffer
    // (the engine recreates dynamic vertex/index buffers mid-frame, which calls
    // Buffer::Create -> Clear on the live buffer). The LogicalDevice frees the
    // handles once the owning frame's fence has signalled. See LogicalDevice's
    // retirement queue and vulkan::Context's frames-in-flight loop.
    if (m_buffer != VK_NULL_HANDLE || m_memory != VK_NULL_HANDLE) {
        m_device->RetireBuffer(m_buffer, m_memory);
        m_buffer = VK_NULL_HANDLE;
        m_memory = VK_NULL_HANDLE;
    }
    m_size = 0;
}

}
}
