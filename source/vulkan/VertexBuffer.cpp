#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/Utility.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

VertexBuffer::VertexBuffer(const std::shared_ptr<LogicalDevice>& device)
    : m_device(device)
{
}

VertexBuffer::~VertexBuffer()
{
    vkDestroyBuffer(m_device->GetHandler(), m_buffer, NULL);
    vkFreeMemory(m_device->GetHandler(), m_memory, NULL);
}

int VertexBuffer::GetSizeInBytes() const
{
	return 0;
}

BufferUsageHint VertexBuffer::GetUsageHint() const
{
	return BufferUsageHint::StreamDraw;
}

void VertexBuffer::ReadFromMemory(const void* data, int size, int offset)
{
}

void* VertexBuffer::WriteToMemory(int size, int offset)
{
	return nullptr;
}

void VertexBuffer::Bind() const
{
}

void VertexBuffer::UnBind()
{
}

void VertexBuffer::Reset(int size_in_bytes)
{
}

void VertexBuffer::Create(const PhysicalDevice& phy_dev, const void* data, 
                          size_t size, size_t stride, bool use_texture)
{
    VkResult res;

    auto vk_dev = m_device->GetHandler();

    m_vertex_count = size / stride;

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buf_info.size = size;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    res = vkCreateBuffer(vk_dev, &buf_info, NULL, &m_buffer);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(vk_dev, m_buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = PhysicalDevice::FindMemoryType(
        phy_dev.GetHandler(), mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    res = vkAllocateMemory(vk_dev, &alloc_info, NULL, &m_memory);
    assert(res == VK_SUCCESS);
    m_info.range = mem_reqs.size;
    m_info.offset = 0;

    uint8_t *pData;
    res = vkMapMemory(vk_dev, m_memory, 0, mem_reqs.size, 0, (void **)&pData);
    assert(res == VK_SUCCESS);

    memcpy(pData, data, size);

    vkUnmapMemory(vk_dev, m_memory);

    res = vkBindBufferMemory(vk_dev, m_buffer, m_memory, 0);
    assert(res == VK_SUCCESS);

    m_vi_binding.binding = 0;
    m_vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_vi_binding.stride = stride;

    m_vi_attribs[0].binding = 0;
    m_vi_attribs[0].location = 0;
    m_vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    m_vi_attribs[0].offset = 0;
    m_vi_attribs[1].binding = 0;
    m_vi_attribs[1].location = 1;
    m_vi_attribs[1].format = use_texture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
    m_vi_attribs[1].offset = 16;
}

}
}