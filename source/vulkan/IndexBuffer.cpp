#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/Utility.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

IndexBuffer::IndexBuffer(VkDevice device)
	: m_device(device)
{
}

IndexBuffer::~IndexBuffer()
{
	vkDestroyBuffer(m_device, m_buffer, NULL);
	vkFreeMemory(m_device, m_memory, NULL);
}

int IndexBuffer::GetSizeInBytes() const
{
	return 0;
}

BufferUsageHint IndexBuffer::GetUsageHint() const
{
    return BufferUsageHint::StreamDraw;
}

IndexBufferDataType IndexBuffer::GetDataType() const
{
    return IndexBufferDataType::UnsignedShort;
}

void IndexBuffer::ReadFromMemory(const void* data, int size, int offset)
{
}

void* IndexBuffer::WriteToMemory(int size, int offset)
{
	return nullptr;
}

void IndexBuffer::Bind() const
{
}

void IndexBuffer::UnBind()
{
}

void IndexBuffer::Reset(int size_in_bytes)
{
}

void IndexBuffer::Create(VkPhysicalDevice phy_dev, const void* data, size_t size)
{
	// Index buffer
	VkBufferCreateInfo indexbufferInfo = {};
	indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexbufferInfo.size = size;
	indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	// Copy index data to a buffer visible to the host
	auto res = vkCreateBuffer(m_device, &indexbufferInfo, nullptr, &m_buffer);
	assert(res == VK_SUCCESS);

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(m_device, m_buffer, &mem_reqs);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = 0;
	alloc_info.memoryTypeIndex = PhysicalDevice::FindMemoryType(
		phy_dev, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	res = vkAllocateMemory(m_device, &alloc_info, nullptr, &m_memory);
	assert(res == VK_SUCCESS);

	uint8_t* pData;
	res = vkMapMemory(m_device, m_memory, 0, size, 0, (void**)&pData);
	assert(res == VK_SUCCESS);

	memcpy(pData, data, size);

	vkUnmapMemory(m_device, m_memory);

	res = vkBindBufferMemory(m_device, m_buffer, m_memory, 0);
	assert(res == VK_SUCCESS);
}

}
}