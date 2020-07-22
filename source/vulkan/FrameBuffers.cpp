#include "unirender/vulkan/FrameBuffers.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/VulkanContext.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

FrameBuffers::FrameBuffers(VkDevice device)
	: m_device(device)
{
}

FrameBuffers::~FrameBuffers()
{
	for (auto& buf : m_frame_buffers) {
		vkDestroyFramebuffer(m_device, buf, nullptr);
	}
}

void FrameBuffers::Create(const VulkanContext& vk_ctx, bool include_depth)
{
    /* DEPENDS on init_depth_buffer(), init_renderpass() and
     * init_swapchain_extension() */

    VkResult res;
    VkImageView attachments[2];
    attachments[1] = vk_ctx.GetDepthBuffer()->GetView();

    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.renderPass = vk_ctx.GetRenderPass()->GetHandler();
    fb_info.attachmentCount = include_depth ? 2 : 1;
    fb_info.pAttachments = attachments;
    fb_info.width = vk_ctx.GetWidth();
    fb_info.height = vk_ctx.GetHeight();
    fb_info.layers = 1;

    auto count = vk_ctx.GetSwapchain()->GetImageCount();
    m_frame_buffers.resize(count);
    auto device = vk_ctx.GetLogicalDevice()->GetHandler();
    for (size_t i = 0; i < count; ++i) 
    {
        attachments[0] = vk_ctx.GetSwapchain()->GetView(i);
        res = vkCreateFramebuffer(device, &fb_info, NULL, &m_frame_buffers[i]);
        assert(res == VK_SUCCESS);
    }
}

}
}