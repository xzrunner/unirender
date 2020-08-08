#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/CommandPool.h"

#include <assert.h>

namespace
{

VkFormat to_attr_format(ur::ComponentDataType type, size_t num)
{
    switch (type)
    {
    case ur::ComponentDataType::Float:
    {
        switch (num)
        {
        case 1:
            return VK_FORMAT_R32_SFLOAT;
        case 2:
            return VK_FORMAT_R32G32_SFLOAT;
        case 3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case 4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:
            assert(0);
        }
    }
        break;
    case ur::ComponentDataType::UnsignedByte:
    {
        switch (num)
        {
        case 1:
            return VK_FORMAT_R8_UNORM;
        case 2:
            return VK_FORMAT_R8G8_UNORM;
        case 3:
            return VK_FORMAT_R8G8B8_UNORM;
        case 4:
            return VK_FORMAT_R8G8B8A8_UNORM;
        default:
            assert(0);
        }
    }
        break;
    default:
        assert(0);
    }
    return VK_FORMAT_UNDEFINED;
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
    m_vi_binding.binding = 0;
    m_vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
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
    Buffer staging(m_device);
    staging.Create(m_phy_dev->GetHandler(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    staging.Upload(data, size);
    m_buffer.Create(m_phy_dev->GetHandler(), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_buffer.CopyFrom(staging, size, m_cmd_pool->GetHandler(), m_device->GetGraphicsQueue());

    //m_buffer.Create(m_phy_dev->GetHandler(), size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    //    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //m_buffer.Upload(data, size);
    
    if (m_vi_binding.stride == 0) {
        int zz = 0;
    }

    assert(m_vi_binding.stride != 0);
    m_vertex_count = size / m_vi_binding.stride;
    
    //m_info.range = mem_reqs.size;
    //m_info.offset = 0;
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

void VertexBuffer::SetVertInputAttrDesc(const std::vector<std::shared_ptr<ur::VertexInputAttribute>>& attrs)
{
    if (!attrs.empty()) {
        m_vi_binding.stride = attrs.front()->GetStrideInBytes();
    }

    m_vi_attribs.resize(attrs.size());
    for (size_t i = 0, n = attrs.size(); i < n; ++i)
    {
        auto& src = attrs[i];
        auto& dst = m_vi_attribs[i];
        dst.binding = 0;
        dst.location = i;
        dst.format = to_attr_format(src->GetCompDataType(), src->GetNumOfComps());
        dst.offset = src->GetOffsetInBytes();
    }
}

}
}