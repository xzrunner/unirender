#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/CommandPool.h"

#include <cassert>

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
    return m_data_type;
}

void IndexBuffer::ReadFromMemory(const void* data, int size, int offset)
{
    if (!data || size <= 0) return;

    // Stream/dynamic indices (re-uploaded every frame): write DIRECTLY into a
    // HOST_VISIBLE | HOST_COHERENT index buffer -- no staging buffer, no copy, no
    // synchronous submit. See VertexBuffer::ReadFromMemory for the full rationale
    // (frames-in-flight stall avoidance, unified-memory, deferred-destroy safety).
    m_buffer.Create(m_phy_dev->GetHandler(), size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_buffer.Upload(data, size);

    m_size_in_bytes = size;

    // FIX: compute element count from data size and data type
    size_t elem_size = (m_data_type == IndexBufferDataType::UnsignedInt) ? 4 : 2;
    m_count = static_cast<uint32_t>(size / elem_size);
}

void* IndexBuffer::WriteToMemory(int size, int offset)
{
    return nullptr; // TODO: map for write-back
}

void IndexBuffer::Bind() const
{
    // Binding is done in Context::BuildCommandBuffer via vkCmdBindIndexBuffer
}

void IndexBuffer::UnBind()
{
    // No-op in Vulkan
}

void IndexBuffer::Reserve(int size_in_bytes)
{
    if (size_in_bytes <= 0) return;

    m_buffer.Create(m_phy_dev->GetHandler(), size_in_bytes,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_size_in_bytes = size_in_bytes;
}

void IndexBuffer::SetDataType(IndexBufferDataType data_type)
{
    m_data_type = data_type;
}

void IndexBuffer::Create(const PhysicalDevice& phy_dev, const void* data, size_t size)
{
    ReadFromMemory(data, static_cast<int>(size), 0);
}

}
}
