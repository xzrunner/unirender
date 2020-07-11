#include "unirender/vulkan/FrameBuffers.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/DeviceInfo.h"
#include "unirender/vulkan/ContextInfo.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/Swapchain.h"

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

void FrameBuffers::Create(const DeviceInfo& dev_info, const ContextInfo& ctx_info,
                          bool include_depth)
{
    /* DEPENDS on init_depth_buffer(), init_renderpass() and
     * init_swapchain_extension() */

    VkResult res;
    VkImageView attachments[2];
    attachments[1] = ctx_info.depth_buf->GetView();

    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.renderPass = ctx_info.renderpass->GetHandler();
    fb_info.attachmentCount = include_depth ? 2 : 1;
    fb_info.pAttachments = attachments;
    fb_info.width = ctx_info.width;
    fb_info.height = ctx_info.height;
    fb_info.layers = 1;

    auto count = ctx_info.swapchain->GetImageCount();
    m_frame_buffers.resize(count);
    for (int i = 0; i < count; ++i) {
        attachments[0] = ctx_info.swapchain->GetView(i);
        res = vkCreateFramebuffer(dev_info.device, &fb_info, NULL, &m_frame_buffers[i]);
        assert(res == VK_SUCCESS);
    }
}

}
}