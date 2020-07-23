#pragma once

#include "unirender/IndexBuffer.h"

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class IndexBuffer : public ur::IndexBuffer
{
public:
    IndexBuffer(VkDevice device);
    virtual ~IndexBuffer();

    virtual int GetSizeInBytes() const override;
    virtual BufferUsageHint GetUsageHint() const override;
    virtual IndexBufferDataType GetDataType() const override;

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    static void UnBind();

    virtual void Reset(int size_in_bytes) override;

    virtual void SetDataType(IndexBufferDataType data_type) override {}

    void Create(VkPhysicalDevice phy_dev, const void* data, size_t size);

    auto& GetBuffer() const { return m_buffer; }
    auto GetCount() const { return m_count; }

private:
    VkDevice m_device = VK_NULL_HANDLE;

    VkDeviceMemory m_memory;
    VkBuffer       m_buffer;
    uint32_t       m_count;

}; // IndexBuffer

}
}