#pragma once

#include "unirender/Framebuffer.h"
#include "unirender/AttachmentType.h"
#include "unirender/TextureTarget.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

namespace ur
{

class Texture;
class RenderBuffer;

namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;
class DepthBuffer;

// Offscreen render target (render-to-texture). Mirrors metal::Framebuffer: a
// passive container of attachments. The concrete VkRenderPass + VkFramebuffer are
// built lazily (EnsureBuilt) from the attached textures, because the engine reuses
// one Framebuffer object with different destination textures between draws (dtex).
//
// NOTE: distinct from vulkan::FrameBuffers (plural) which wraps the swapchain.
class Framebuffer : public ur::Framebuffer
{
public:
    Framebuffer(const std::shared_ptr<LogicalDevice>& device,
        const std::shared_ptr<PhysicalDevice>& phy_dev = nullptr);
    virtual ~Framebuffer();

    virtual void SetAttachment(AttachmentType type, TextureTarget target,
        const std::shared_ptr<ur::Texture>& tex, const std::shared_ptr<ur::RenderBuffer>& rbo,
        int mipmap_level = 0) override;

    virtual void Bind() const override {}

    struct Attachment
    {
        AttachmentType type    = AttachmentType::Color0;
        TextureTarget  target  = TextureTarget::Texture2D;
        int            mipmap_level = 0;
        std::shared_ptr<ur::Texture>      tex;
        std::shared_ptr<ur::RenderBuffer> rbo;
    };
    const std::vector<Attachment>& GetAttachments() const { return m_attachments; }

    // Build (or rebuild, if the bound color view changed) the offscreen
    // VkRenderPass + VkFramebuffer over the current color attachment.
    void EnsureBuilt();

    VkRenderPass  GetVkRenderPass()  const { return m_render_pass; }
    VkFramebuffer GetVkFramebuffer() const { return m_framebuffer; }
    VkExtent2D    GetExtent()        const { return m_extent; }
    // Number of color attachments the built render pass has (MRT GBuffer = 3). The
    // pipeline's color-blend state must declare the same count.
    uint32_t      GetColorAttachmentCount() const { return (uint32_t)m_built_views.size(); }
    // True when the built render pass has a depth attachment (GBuffer). The offscreen
    // pipeline must enable depth test/write only when this is true.
    bool          HasDepth() const { return m_depth != nullptr; }

private:
    void Destroy();

private:
    std::shared_ptr<LogicalDevice>  m_device;
    std::shared_ptr<PhysicalDevice> m_phy_dev;

    std::vector<Attachment> m_attachments;

    VkRenderPass  m_render_pass = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    VkExtent2D    m_extent      = { 0, 0 };
    std::vector<VkImageView> m_built_views; // cache key: rebuild when this set changes

    std::shared_ptr<DepthBuffer> m_depth; // owned offscreen depth attachment (GBuffer)

}; // Framebuffer

}
}
