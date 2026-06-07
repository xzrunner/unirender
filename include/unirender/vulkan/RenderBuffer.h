#pragma once

#include "unirender/RenderBuffer.h"
#include "unirender/InternalFormat.h"

namespace ur
{
namespace vulkan
{

// Minimal render buffer. The offscreen vulkan::Framebuffer creates the actual depth
// image itself (sized to its color attachments), so this just needs to be a non-null
// object carrying the requested format/size -- otherwise the rendergraph's RenderTarget
// node skips attaching depth (render_target.ves: `if (!rbo) return`) and the GBuffer
// gets no depth buffer (the 3D mesh then renders with no depth test -> see-through).
class RenderBuffer : public ur::RenderBuffer
{
public:
    RenderBuffer(int width, int height, ur::InternalFormat format)
        : m_width(width), m_height(height), m_format(format) {}

    virtual void Bind(AttachmentType attach) const override {}

    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }
    ur::InternalFormat GetFormat() const { return m_format; }

    bool IsDepth() const {
        return m_format == ur::InternalFormat::DepthComponent
            || m_format == ur::InternalFormat::DepthStencil;
    }

private:
    int m_width  = 0;
    int m_height = 0;
    ur::InternalFormat m_format = ur::InternalFormat::RGBA;

}; // RenderBuffer

}
}
