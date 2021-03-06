#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>


namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;

class DepthBuffer : noncopyable
{
public:
    DepthBuffer(const std::shared_ptr<LogicalDevice>& device, 
        const PhysicalDevice& phy_dev, int width, int height);
    ~DepthBuffer();

    auto GetFormat() const { return m_format; }
    auto GetView() const { return m_view; }

private:
    std::shared_ptr<LogicalDevice> m_device = nullptr;

    VkFormat m_format;

    VkImage        m_image;
    VkDeviceMemory m_mem;
    VkImageView    m_view;

}; // DepthBuffer

}
}