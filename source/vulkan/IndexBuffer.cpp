#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Utility.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

IndexBuffer::IndexBuffer(const std::shared_ptr<LogicalDevice>& device)
	: m_device(device)
{
}

IndexBuffer::~IndexBuffer()
{
	vkDestroyBuffer(m_device->GetHandler(), m_buffer, NULL);
	vkFreeMemory(m_device->GetHandler(), m_memory, NULL);
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

void IndexBuffer::Create(const PhysicalDevice& phy_dev, const void* data, size_t size)
{
	auto vk_dev = m_device->GetHandler();
	Utility::CreateBuffer(vk_dev, phy_dev.GetHandler(), size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_memory);

	uint8_t* buf;
	if (vkMapMemory(vk_dev, m_memory, 0, size, 0, (void**)&buf) != VK_SUCCESS) {
		throw std::runtime_error("failed to map memory!");
	}
	memcpy(buf, data, size);
	vkUnmapMemory(vk_dev, m_memory);
}

}
}