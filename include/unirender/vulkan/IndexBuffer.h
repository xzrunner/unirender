#pragma once

#include "unirender/IndexBuffer.h"
#include "unirender/vulkan/Buffer.h"

#include <vulkan/vulkan.h>

#include <memory>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;
class CommandPool;

class IndexBuffer : public ur::IndexBuffer
{
public:
    IndexBuffer(const std::shared_ptr<LogicalDevice>& device,
        const std::shared_ptr<PhysicalDevice>& phy_dev,
        const std::shared_ptr<CommandPool>& cmd_pool);

    virtual BufferUsageHint GetUsageHint() const override;
    virtual IndexBufferDataType GetDataType() const override;

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    static void UnBind();

    virtual void Reserve(int size_in_bytes) override;

    virtual void SetDataType(IndexBufferDataType data_type) override {}

    void Create(const PhysicalDevice& phy_dev, const void* data, size_t size);

    auto GetBuffer() const { return m_buffer.GetHandler(); }
    auto GetCount() const { return m_count; }

private:
    std::shared_ptr<LogicalDevice>  m_device   = nullptr;
    std::shared_ptr<PhysicalDevice> m_phy_dev  = nullptr;
    std::shared_ptr<CommandPool>    m_cmd_pool = nullptr;

    Buffer m_buffer;

    uint32_t m_count  = 0;

}; // IndexBuffer

}
}