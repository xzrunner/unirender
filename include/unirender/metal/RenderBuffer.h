#pragma once

#include "unirender/RenderBuffer.h"
#include "unirender/AttachmentType.h"
#include "unirender/InternalFormat.h"

namespace ur
{
namespace metal
{

class RenderBuffer : public ur::RenderBuffer
{
public:
    RenderBuffer(void* mtl_device, int width, int height, InternalFormat format);
    virtual ~RenderBuffer();

    virtual void Bind(AttachmentType attach) const override {}
    static void UnBind() {}

    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }

    void* GetMTLTexture() const { return m_mtl_texture; }

private:
    void* m_mtl_device  = nullptr;
    void* m_mtl_texture = nullptr; // id<MTLTexture> used as render target

    int m_width  = 0;
    int m_height = 0;
    InternalFormat m_format;

}; // RenderBuffer

}
}
