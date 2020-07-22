#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class VulkanContext;

class RenderPass
{
public:
    RenderPass(VkDevice device);
    ~RenderPass();

    void Create(const VulkanContext& vk_ctx, 
        bool include_depth, bool clear = true, 
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

    auto GetHandler() const { return m_handle; }

private:
    VkDevice m_device = VK_NULL_HANDLE;

	VkRenderPass m_handle = VK_NULL_HANDLE;

}; // RenderPass

}
}