#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/DeviceInfo.h"
#include "unirender/vulkan/Utility.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

VertexBuffer::VertexBuffer()
{
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

void VertexBuffer::Create(const DeviceInfo& dev_info, const void* data, size_t size, size_t stride, bool use_texture)
{
    VkResult res;
    bool pass;

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buf_info.size = size;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    res = vkCreateBuffer(dev_info.device, &buf_info, NULL, &m_vertex_buffer.buf);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(dev_info.device, m_vertex_buffer.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    pass = Utility::MemoryTypeFromProperties(dev_info, mem_reqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &alloc_info.memoryTypeIndex);
    assert(pass && "No mappable, coherent memory");

    res = vkAllocateMemory(dev_info.device, &alloc_info, NULL, &(m_vertex_buffer.mem));
    assert(res == VK_SUCCESS);
    m_vertex_buffer.buffer_info.range = mem_reqs.size;
    m_vertex_buffer.buffer_info.offset = 0;

    uint8_t *pData;
    res = vkMapMemory(dev_info.device, m_vertex_buffer.mem, 0, mem_reqs.size, 0, (void **)&pData);
    assert(res == VK_SUCCESS);

    memcpy(pData, data, size);

    vkUnmapMemory(dev_info.device, m_vertex_buffer.mem);

    res = vkBindBufferMemory(dev_info.device, m_vertex_buffer.buf, m_vertex_buffer.mem, 0);
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