#include "unirender/vulkan/Framebuffer.h"
#include "unirender/vulkan/Texture.h"
#include "unirender/vulkan/RenderBuffer.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <stdexcept>

namespace ur
{
namespace vulkan
{

Framebuffer::Framebuffer(const std::shared_ptr<LogicalDevice>& device,
                         const std::shared_ptr<PhysicalDevice>& phy_dev)
    : m_device(device)
    , m_phy_dev(phy_dev)
{
}

Framebuffer::~Framebuffer()
{
    Destroy();
}

void Framebuffer::Destroy()
{
    if (!m_device) { return; }
    auto dev = m_device->GetHandler();
    if (m_framebuffer) { vkDestroyFramebuffer(dev, m_framebuffer, nullptr); m_framebuffer = VK_NULL_HANDLE; }
    if (m_render_pass) { vkDestroyRenderPass(dev, m_render_pass, nullptr); m_render_pass = VK_NULL_HANDLE; }
    m_depth.reset();
}

void Framebuffer::SetAttachment(AttachmentType type, TextureTarget target,
                                const std::shared_ptr<ur::Texture>& tex,
                                const std::shared_ptr<ur::RenderBuffer>& rbo, int mipmap_level)
{
    for (auto& a : m_attachments) {
        if (a.type == type) {
            a = { type, target, mipmap_level, tex, rbo };
            return;
        }
    }
    m_attachments.push_back({ type, target, mipmap_level, tex, rbo });
}

void Framebuffer::EnsureBuilt()
{
    // Collect ALL color attachments in slot order (Color0, Color1, ...). The deferred
    // GBuffer is MRT: gMain(loc0)/gDepth(loc1)/gNormal(loc2) -> attachments 0/1/2, so the
    // mesh fragment shader's 3 outputs all get written.
    std::vector<vulkan::Texture*> colors;
    std::vector<VkImageView>      views;
    for (int slot = (int)AttachmentType::Color0; slot <= (int)AttachmentType::Color15; ++slot) {
        for (auto& a : m_attachments) {
            if ((int)a.type == slot && a.tex) {
                auto* t = static_cast<vulkan::Texture*>(a.tex.get());
                VkImageView v = t->GetImageView();
                if (v != VK_NULL_HANDLE) { colors.push_back(t); views.push_back(v); }
                break;
            }
        }
    }
    if (colors.empty()) { return; }

    // Does this FBO want a depth attachment (GBuffer)? The rendergraph attaches a depth
    // renderbuffer; we create our own depth image sized to the color extent.
    bool want_depth = false;
    for (auto& a : m_attachments) {
        if (a.type == AttachmentType::Depth && (a.rbo || a.tex)) { want_depth = true; break; }
    }
    want_depth = want_depth && (m_phy_dev != nullptr);

    if (m_framebuffer != VK_NULL_HANDLE && views == m_built_views &&
        (bool)m_depth == want_depth) { return; } // already built

    Destroy();

    auto dev = m_device->GetHandler();
    m_extent = { static_cast<uint32_t>(colors[0]->GetWidth()), static_cast<uint32_t>(colors[0]->GetHeight()) };

    if (want_depth) {
        m_depth = std::make_shared<DepthBuffer>(m_device, *m_phy_dev,
            (int)m_extent.width, (int)m_extent.height);
    }

    const uint32_t n = (uint32_t)colors.size();
    std::vector<VkAttachmentDescription> atts(n);
    std::vector<VkAttachmentReference>   refs(n);
    for (uint32_t i = 0; i < n; ++i) {
        // The targets live in SHADER_READ_ONLY_OPTIMAL between passes (creation +
        // each pass's finalLayout), so LOAD preserves prior content (e.g. dtex packs
        // glyphs into the atlas without wiping it).
        atts[i].format         = colors[i]->GetVkFormat();
        atts[i].samples        = VK_SAMPLE_COUNT_1_BIT;
        atts[i].loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD;
        atts[i].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        atts[i].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        atts[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        atts[i].initialLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        atts[i].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        refs[i].attachment = i;
        refs[i].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    // Framebuffer attachment views = color views (+ the depth view last). m_built_views
    // stays color-only (cache key + GetColorAttachmentCount).
    std::vector<VkImageView> fb_views = views;

    // Depth attachment (GBuffer): cleared each pass (loadOp=CLEAR), not stored/sampled
    // -- the edge pass reads gDepth (a COLOR attachment), the depth buffer is only for
    // the mesh pass's depth test.
    VkAttachmentReference depth_ref = {};
    if (want_depth) {
        VkAttachmentDescription d = {};
        d.format         = m_depth->GetFormat();
        d.samples        = VK_SAMPLE_COUNT_1_BIT;
        d.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        d.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        d.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        d.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        d.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        d.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        atts.push_back(d);
        depth_ref.attachment = n; // after the color attachments
        depth_ref.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        fb_views.push_back(m_depth->GetView());
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = n;
    subpass.pColorAttachments       = refs.data();
    subpass.pDepthStencilAttachment = want_depth ? &depth_ref : nullptr;

    // Order the offscreen color writes before a later screen pass samples the
    // texture, and after any prior sampling of it. Include the depth stages so the
    // depth clear/test is synchronized too.
    VkSubpassDependency deps[2] = {};
    deps[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
    deps[0].dstSubpass    = 0;
    deps[0].srcStageMask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    deps[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    deps[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    deps[1].srcSubpass    = 0;
    deps[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
    deps[1].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    deps[1].dstStageMask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    deps[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkRenderPassCreateInfo rp_ci = {};
    rp_ci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_ci.attachmentCount = (uint32_t)atts.size();
    rp_ci.pAttachments    = atts.data();
    rp_ci.subpassCount    = 1;
    rp_ci.pSubpasses      = &subpass;
    rp_ci.dependencyCount = 2;
    rp_ci.pDependencies   = deps;
    if (vkCreateRenderPass(dev, &rp_ci, nullptr, &m_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen render pass!");
    }

    VkFramebufferCreateInfo fb_ci = {};
    fb_ci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_ci.renderPass      = m_render_pass;
    fb_ci.attachmentCount = (uint32_t)fb_views.size();
    fb_ci.pAttachments    = fb_views.data();
    fb_ci.width           = m_extent.width;
    fb_ci.height          = m_extent.height;
    fb_ci.layers          = 1;
    if (vkCreateFramebuffer(dev, &fb_ci, nullptr, &m_framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create offscreen framebuffer!");
    }

    m_built_views = views;
}

}
}
