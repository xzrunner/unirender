#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class DeviceInfo;
class ContextInfo;

class DepthBuffer
{
public:
    DepthBuffer(VkDevice device);
    ~DepthBuffer();

	void Create(const DeviceInfo& dev_info, 
        const ContextInfo& ctx_info);

    auto GetFormat() const { return m_format; }
    auto GetView() const { return m_view; }

private:
    VkDevice m_device;

    VkFormat m_format;

    VkImage        m_image;
    VkDeviceMemory m_mem;
    VkImageView    m_view;

}; // DepthBuffer

}
}