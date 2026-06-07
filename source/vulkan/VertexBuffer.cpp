#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/VertexInputAttribute.h"

#include <cassert>

namespace
{

VkFormat to_attr_format(ur::ComponentDataType type, size_t num)
{
    switch (type)
    {
    case ur::ComponentDataType::Float:
        switch (num)
        {
        case 1: return VK_FORMAT_R32_SFLOAT;
        case 2: return VK_FORMAT_R32G32_SFLOAT;
        case 3: return VK_FORMAT_R32G32B32_SFLOAT;
        case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        default: assert(0); return VK_FORMAT_UNDEFINED;
        }
    case ur::ComponentDataType::Int:
        switch (num)
        {
        case 1: return VK_FORMAT_R32_SINT;
        case 2: return VK_FORMAT_R32G32_SINT;
        case 3: return VK_FORMAT_R32G32B32_SINT;
        case 4: return VK_FORMAT_R32G32B32A32_SINT;
        default: assert(0); return VK_FORMAT_UNDEFINED;
        }
    case ur::ComponentDataType::UnsignedByte:
        switch (num)
        {
        case 1: return VK_FORMAT_R8_UNORM;
        case 2: return VK_FORMAT_R8G8_UNORM;
        case 3: return VK_FORMAT_R8G8B8_UNORM;
        case 4: return VK_FORMAT_R8G8B8A8_UNORM;
        default: assert(0); return VK_FORMAT_UNDEFINED;
        }
    default:
        assert(0);
        return VK_FORMAT_UNDEFINED;
    }
}

}

namespace ur
{
namespace vulkan
{

VertexBuffer::VertexBuffer(const std::shared_ptr<LogicalDevice>& device,
                           const std::shared_ptr<PhysicalDevice>& phy_dev,
                           const std::shared_ptr<CommandPool>& cmd_pool)
    : m_device(device)
    , m_phy_dev(phy_dev)
    , m_cmd_pool(cmd_pool)
    , m_buffer(device)
{
    m_vi_binding.binding   = 0;
    m_vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_vi_binding.stride    = 0; // set when vertex attrs are configured
}

int VertexBuffer::GetSizeInBytes() const
{
    // FIX: return actual buffer size
    return m_size_in_bytes;
}

BufferUsageHint VertexBuffer::GetUsageHint() const
{
    return BufferUsageHint::StreamDraw;
}

void VertexBuffer::ReadFromMemory(const void* data, int size, int offset)
{
    if (!data || size <= 0) return;

    // Stream/dynamic geometry (GetUsageHint() == StreamDraw, re-uploaded every frame
    // by e.g. SpriteRenderer::Flush): write the data DIRECTLY into a HOST_VISIBLE |
    // HOST_COHERENT vertex buffer. This drops the old staging buffer + vkCmdCopyBuffer
    // + synchronous single-time submit (which, with frames-in-flight, would stall the
    // CPU on prior in-flight frames via the shared graphics queue). Host-coherent
    // writes done before the frame's vkQueueSubmit are visible to that submission, so
    // no barrier is needed. On unified-memory GPUs (MoltenVK/Apple) host-visible
    // memory is also device-local, so there is no GPU read penalty either.
    // Recreating the buffer here is safe: Buffer::Create -> Clear retires the previous
    // VkBuffer for deferred destruction once the frame that referenced it completes.
    m_buffer.Create(m_phy_dev->GetHandler(), size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_buffer.Upload(data, size);

    // FIX: track buffer size
    m_size_in_bytes = size;

    // FIX: recompute vertex count if stride is known
    if (m_vi_binding.stride > 0) {
        m_vertex_count = static_cast<size_t>(size) / m_vi_binding.stride;
    }
}

void* VertexBuffer::WriteToMemory(int size, int offset)
{
    return nullptr; // TODO: map for CPU write
}

void VertexBuffer::Bind() const
{
    // Binding is done in Context::BuildCommandBuffer via vkCmdBindVertexBuffers
}

void VertexBuffer::UnBind()
{
    // No-op in Vulkan
}

void VertexBuffer::Reserve(int size_in_bytes)
{
    if (size_in_bytes <= 0) return;

    m_buffer.Create(m_phy_dev->GetHandler(), size_in_bytes,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_size_in_bytes = size_in_bytes;
}

void VertexBuffer::SetVertInputAttrDesc(
    const std::vector<std::shared_ptr<ur::VertexInputAttribute>>& attrs)
{
    m_vi_attribs.clear();
    m_vi_attribs.reserve(attrs.size());

    uint32_t stride = 0;
    for (size_t i = 0; i < attrs.size(); ++i)
    {
        if (!attrs[i]) continue;

        VkVertexInputAttributeDescription desc;
        desc.location = static_cast<uint32_t>(attrs[i]->GetLocation());
        desc.binding  = 0;
        desc.format   = to_attr_format(attrs[i]->GetCompDataType(),
                                       attrs[i]->GetNumOfComps());
        desc.offset   = static_cast<uint32_t>(attrs[i]->GetOffsetInBytes());
        m_vi_attribs.push_back(desc);

        // Track maximum stride
        uint32_t attr_stride = static_cast<uint32_t>(attrs[i]->GetStrideInBytes());
        if (attr_stride > stride) {
            stride = attr_stride;
        }
    }

    m_vi_binding.stride = stride;

    // FIX: recompute vertex count if data is already uploaded
    if (m_size_in_bytes > 0 && stride > 0) {
        m_vertex_count = static_cast<size_t>(m_size_in_bytes) / stride;
    }
}

}
}
