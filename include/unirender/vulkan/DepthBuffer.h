#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class ContextInfo;

class DepthBuffer
{
public:
    DepthBuffer(VkDevice device);
    ~DepthBuffer();

	void Create(const ContextInfo& ctx_info);

    auto GetFormat() const { return m_format; }
    auto GetView() const { return m_view; }

private:
    VkDevice m_device = VK_NULL_HANDLE;

    VkFormat m_format;

    VkImage        m_image;
    VkDeviceMemory m_mem;
    VkImageView    m_view;

}; // DepthBuffer

}
}