#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/Surface.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/Context.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

namespace ur
{
namespace vulkan
{

RenderPass::RenderPass(const Context& ctx, bool include_depth, bool clear,
                       VkImageLayout finalLayout, VkImageLayout initialLayout)
	: m_device(ctx.GetLogicalDevice())
{
    /* DEPENDS on init_swap_chain() and init_depth_buffer() */

	assert(clear || (initialLayout != VK_IMAGE_LAYOUT_UNDEFINED));

    Swapchain::SwapChainSupportDetails swapChainSupport
        = Swapchain::QuerySwapChainSupport(ctx.GetPhysicalDevice()->GetHandler(), ctx.GetSurface()->GetHandler());
    VkSurfaceFormatKHR surfaceFormat = Swapchain::ChooseSwapSurfaceFormat(swapChainSupport.formats);

    VkResult res;
    /* Need attachments for render target and depth buffer */
    VkAttachmentDescription attachments[2];
    attachments[0].format = surfaceFormat.format;
    attachments[0].samples = NUM_SAMPLES;
    attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = initialLayout;
    attachments[0].finalLayout = finalLayout;
    attachments[0].flags = 0;

    if (include_depth) {
        attachments[1].format = ctx.GetDepthBuffer()->GetFormat();
        attachments[1].samples = NUM_SAMPLES;
        attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[1].flags = 0;
    }

    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = include_depth ? &depth_reference : NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    // Subpass dependency to wait for wsi image acquired semaphore before starting layout transition
    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext = NULL;
    rp_info.attachmentCount = include_depth ? 2 : 1;
    rp_info.pAttachments = attachments;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    rp_info.dependencyCount = 1;
    rp_info.pDependencies = &subpass_dependency;

    res = vkCreateRenderPass(m_device->GetHandler(), &rp_info, NULL, &m_handle);
    assert(res == VK_SUCCESS);
}

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(m_device->GetHandler(), m_handle, NULL);
}

}
}