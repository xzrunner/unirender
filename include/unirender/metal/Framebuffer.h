#pragma once

#include "unirender/Framebuffer.h"

#include <memory>
#include <vector>

namespace ur
{
namespace metal
{

class Framebuffer : public ur::Framebuffer
{
public:
    Framebuffer(void* mtl_device);
    virtual ~Framebuffer();

    virtual void SetAttachment(AttachmentType type, TextureTarget target,
        const std::shared_ptr<ur::Texture>& tex,
        const std::shared_ptr<ur::RenderBuffer>& rbo,
        int mipmap_level = 0) override;

    virtual void Bind() const override {}

    struct Attachment
    {
        AttachmentType type   = AttachmentType::Color0;
        TextureTarget  target = TextureTarget::Texture2D;
        int mipmap_level      = 0;
        std::shared_ptr<ur::Texture>      tex = nullptr;
        std::shared_ptr<ur::RenderBuffer> rbo = nullptr;
    };

    const std::vector<Attachment>& GetAttachments() const { return m_attachments; }

private:
    void* m_mtl_device = nullptr;
    std::vector<Attachment> m_attachments;

}; // Framebuffer

}
}
