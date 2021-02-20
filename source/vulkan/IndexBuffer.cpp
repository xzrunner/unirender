#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/CommandPool.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

IndexBuffer::IndexBuffer(const std::shared_ptr<LogicalDevice>& device,
	                     const std::shared_ptr<PhysicalDevice>& phy_dev,
	                     const std::shared_ptr<CommandPool>& cmd_pool)
	: m_device(device)
	, m_phy_dev(phy_dev)
	, m_cmd_pool(cmd_pool)
	, m_buffer(device)
{
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
	//m_buffer.Create(phy_dev.GetHandler(), size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	//m_buffer.Upload(data, size);

    Buffer staging(m_device);
    staging.Create(m_phy_dev->GetHandler(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    staging.Upload(data, size);
	m_buffer.Create(m_phy_dev->GetHandler(), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	m_buffer.CopyFrom(staging, size, m_cmd_pool->GetHandler(), m_device->GetGraphicsQueue());
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

void IndexBuffer::Reserve(int size_in_bytes)
{
}

void IndexBuffer::Create(const PhysicalDevice& phy_dev, const void* data, size_t size)
{

}

}
}