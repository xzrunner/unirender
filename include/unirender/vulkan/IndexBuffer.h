#pragma once

#include "unirender/IndexBuffer.h"

#include <vulkan/vulkan.h>

#include <memory>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;

class IndexBuffer : public ur::IndexBuffer
{
public:
    IndexBuffer(const std::shared_ptr<LogicalDevice>& device);
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

    void Create(const PhysicalDevice& phy_dev, const void* data, size_t size);

    auto& GetBuffer() const { return m_buffer; }
    auto GetCount() const { return m_count; }

private:
    std::shared_ptr<LogicalDevice> m_device = nullptr;

    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkBuffer       m_buffer = VK_NULL_HANDLE;
    uint32_t       m_count  = 0;

}; // IndexBuffer

}
}