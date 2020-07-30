#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Utility.h"

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
            return VK_FORMAT_R8_UINT;
        case 2:
            return VK_FORMAT_R8G8_UINT;
        case 3:
            return VK_FORMAT_R8G8B8_UINT;
        case 4:
            return VK_FORMAT_R8G8B8A8_UINT;
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

void VertexBuffer::Create(const PhysicalDevice& phy_dev, const void* data, size_t size, size_t stride)
{
    auto vk_dev = m_device->GetHandler();

    Utility::CreateBuffer(vk_dev, phy_dev.GetHandler(), size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_memory);

    m_vertex_count = size / stride;
    
    //m_info.range = mem_reqs.size;
    //m_info.offset = 0;

    uint8_t *buf;
    if (vkMapMemory(vk_dev, m_memory, 0, size, 0, (void**)&buf) != VK_SUCCESS) {
        throw std::runtime_error("failed to map memory!");
    }
    memcpy(buf, data, size);
    vkUnmapMemory(vk_dev, m_memory);

    m_vi_binding.binding = 0;
    m_vi_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_vi_binding.stride = stride;
}

void VertexBuffer::SetVertInputAttrDesc(const std::vector<std::shared_ptr<ur::VertexBufferAttribute>>& attrs)
{
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