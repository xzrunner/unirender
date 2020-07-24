#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class Context;
class LogicalDevice;

class RenderPass : boost::noncopyable
{
public:
    RenderPass(const Context& ctx, bool include_depth, bool clear = true,
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);
    ~RenderPass();

    auto GetHandler() const { return m_handle; }

private:
    std::shared_ptr<LogicalDevice> m_device = VK_NULL_HANDLE;

	VkRenderPass m_handle = VK_NULL_HANDLE;

}; // RenderPass

}
}