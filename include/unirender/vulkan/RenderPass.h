#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class DeviceInfo;
class ContextInfo;

class RenderPass
{
public:
    RenderPass(VkDevice device);
    ~RenderPass();

    void Create(const DeviceInfo& dev_info, const ContextInfo& ctx_info, 
        bool include_depth, bool clear = true, 
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

    auto GetHandler() const { return m_handle; }

private:
    VkDevice m_device;

	VkRenderPass m_handle = VK_NULL_HANDLE;

}; // RenderPass

}
}