#import <Metal/Metal.h>
#include "unirender/metal/Framebuffer.h"

namespace ur
{
namespace metal
{

Framebuffer::Framebuffer(void* mtl_device)
    : m_mtl_device(mtl_device)
{
}

Framebuffer::~Framebuffer() {}

void Framebuffer::SetAttachment(AttachmentType type, TextureTarget target,
    const std::shared_ptr<ur::Texture>& tex,
    const std::shared_ptr<ur::RenderBuffer>& rbo,
    int mipmap_level)
{
    // Metal FBOs are configured at render-pass begin via MTLRenderPassDescriptor.
    // We store the attachment info here; Context reads it when building the pass.
    Attachment att;
    att.type         = type;
    att.target       = target;
    att.mipmap_level = mipmap_level;
    att.tex          = tex;
    att.rbo          = rbo;

    // Replace if same type already exists
    for (auto& a : m_attachments) {
        if (a.type == type) {
            a = att;
            return;
        }
    }
    m_attachments.push_back(att);
}

}
}
