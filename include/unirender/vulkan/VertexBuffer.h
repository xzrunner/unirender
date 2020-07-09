#pragma once

#include "unirender/VertexBuffer.h"

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class DeviceInfo;

class VertexBuffer : public ur::VertexBuffer
{
public:
    VertexBuffer();

    virtual int GetSizeInBytes() const override;
    virtual BufferUsageHint GetUsageHint() const override;

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    static void UnBind();

    virtual void Reset(int size_in_bytes) override;

    void Create(const DeviceInfo& dev_info, const void* data, size_t size, size_t stride, bool use_texture);

    auto& GetBuffer() const { return m_vertex_buffer.buf; }

    auto& GetVertInputBindDesc() const { return m_vi_binding; }
    auto& GetVertInputAttrDesc() const { return m_vi_attribs; }

    struct {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo buffer_info;
    } m_vertex_buffer;

private:
    VkVertexInputBindingDescription   m_vi_binding;
    VkVertexInputAttributeDescription m_vi_attribs[2];

}; // VertexBuffer

}
}