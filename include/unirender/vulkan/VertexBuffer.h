#pragma once

#include "unirender/VertexBuffer.h"
#include "unirender/VertexBufferAttribute.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;

class VertexBuffer : public ur::VertexBuffer
{
public:
    VertexBuffer(const std::shared_ptr<LogicalDevice>& device);
    virtual ~VertexBuffer();

    virtual int GetSizeInBytes() const override;
    virtual BufferUsageHint GetUsageHint() const override;

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    static void UnBind();

    virtual void Reset(int size_in_bytes) override;

    virtual size_t GetVertexCount() const override { return m_vertex_count; }

    void Create(const PhysicalDevice& phy_dev, const void* data, size_t size, size_t stride);

    auto& GetBuffer() const { return m_buffer; }

    auto& GetVertInputBindDesc() const { return m_vi_binding; }

    void SetVertInputAttrDesc(const std::vector<std::shared_ptr<ur::VertexBufferAttribute>>& attrs);
    auto& GetVertInputAttrDesc() const { return m_vi_attribs; }

private:
    std::shared_ptr<LogicalDevice> m_device = nullptr;

    VkVertexInputBindingDescription m_vi_binding;
    std::vector<VkVertexInputAttributeDescription> m_vi_attribs;

    VkBuffer               m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory         m_memory = VK_NULL_HANDLE;
    //VkDescriptorBufferInfo m_info;

    size_t m_vertex_count = 0;

}; // VertexBuffer

}
}