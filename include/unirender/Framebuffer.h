#pragma once

#include "unirender/AttachmentType.h"
#include "unirender/TextureTarget.h"

#include <memory>

namespace ur
{

class Texture;
class RenderBuffer;

class Framebuffer
{
public:
    virtual ~Framebuffer() {}

    virtual void SetAttachment(AttachmentType type, TextureTarget target,
        const std::shared_ptr<ur::Texture>& tex, const std::shared_ptr<ur::RenderBuffer>& rbo, int mipmap_level = 0) = 0;

    virtual void Bind() const = 0;

}; // Framebuffer

}