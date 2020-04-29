#pragma once

#include "unirender/Framebuffer.h"
#include "unirender/AttachmentType.h"
#include "unirender/TextureTarget.h"
#include "unirender/opengl/opengl.h"

#include <vector>
#include <memory>

namespace ur
{

class Device;
class Texture;
class RenderBuffer;

namespace opengl
{

class Framebuffer : public ur::Framebuffer
{
public:
    Framebuffer(const ur::Device& device);
    virtual ~Framebuffer();

    virtual void SetAttachment(AttachmentType type, TextureTarget target,
        const std::shared_ptr<ur::Texture>& tex, const std::shared_ptr<ur::RenderBuffer>& rbo, int mipmap_level = 0) override;

    virtual void Bind() const override;
    static void UnBind();

    void Clean();

private:
    struct Attachment
    {
        void Attach();

        bool dirty = false;

        AttachmentType type   = AttachmentType::Color0;
        TextureTarget  target = TextureTarget::Texture2D;
        int mipmap_level = 0;

        std::shared_ptr<Texture>      tex = nullptr;
        std::shared_ptr<RenderBuffer> rbo = nullptr;
    };

private:
    GLuint m_id = 0;

    int m_num_col_attachments = 0;
    std::vector<Attachment> m_attachments;

}; // Framebuffer

}
}