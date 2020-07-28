#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/Utility.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

UniformBuffer::UniformBuffer(const std::shared_ptr<LogicalDevice>& device,
                             const PhysicalDevice& phy_dev, const void* data, size_t size)
    : m_device(device)
{
    auto vk_dev = device->GetHandler();

    VkResult res;

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = size;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    res = vkCreateBuffer(vk_dev, &buf_info, NULL, &m_buf);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(vk_dev, m_buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = PhysicalDevice::FindMemoryType(
        phy_dev.GetHandler(), mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    res = vkAllocateMemory(vk_dev, &alloc_info, NULL, &(m_mem));
    assert(res == VK_SUCCESS);

    Update(data, size);

    res = vkBindBufferMemory(vk_dev, m_buf, m_mem, 0);
    assert(res == VK_SUCCESS);

    m_buffer_info.buffer = m_buf;
    m_buffer_info.offset = 0;
    m_buffer_info.range = size;
}

UniformBuffer::~UniformBuffer()
{
    vkDestroyBuffer(m_device->GetHandler(), m_buf, NULL);
    vkFreeMemory(m_device->GetHandler(), m_mem, NULL);
}

void UniformBuffer::Update(const void* data, size_t size)
{
    auto vk_dev = m_device->GetHandler();

    uint8_t* dst;
    VkResult res = vkMapMemory(vk_dev, m_mem, 0, size, 0, (void**)&dst);
    assert(res == VK_SUCCESS);

    memcpy(dst, data, size);

    vkUnmapMemory(vk_dev, m_mem);
}

}
}