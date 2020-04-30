#include "unirender/opengl/Framebuffer.h"
#include "unirender/opengl/Texture.h"
#include "unirender/opengl/RenderBuffer.h"
#include "unirender/opengl/TypeConverter.h"
#include "unirender/Device.h"

#include <assert.h>

namespace ur
{
namespace opengl
{

Framebuffer::Framebuffer(const ur::Device& device)
{
    glGenFramebuffers(1, &m_id);

    const int num = device.GetMaxNumColorAttachments() + 2;
    m_attachments.resize(num);
    for (int i = 0; i < num; ++i) {
        m_attachments[i].type = static_cast<AttachmentType>(i);
    }
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &m_id);
}

void Framebuffer::SetAttachment(AttachmentType type, TextureTarget target,
                                const std::shared_ptr<ur::Texture>& tex,
                                const std::shared_ptr<ur::RenderBuffer>& rbo,
                                int mipmap_level)
{
    size_t idx = 0;
    switch (type)
    {
    case AttachmentType::Depth:
        assert(m_attachments.size() >= 2);
        idx = m_attachments.size() - 2;
        break;
    case AttachmentType::Stencil:
        assert(m_attachments.size() >= 1);
        idx = m_attachments.size() - 1;
        break;
    default:
        assert(type >= AttachmentType::Color0
            && type <= AttachmentType::Color15);
        idx = static_cast<size_t>(type) - static_cast<size_t>(AttachmentType::Color0);
    }
    assert(idx <= m_attachments.size());
    auto& atta = m_attachments[idx];

    atta.dirty = true;

    atta.type   = type;
    atta.target = target;

    atta.tex = tex;
    atta.rbo = rbo;

    atta.mipmap_level = mipmap_level;
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void Framebuffer::UnBind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Clean()
{
    bool dirty = false;
    for (auto& atta : m_attachments) {
        if (atta.dirty) {
            dirty = true;
            break;
        }
    }

    if (!dirty) {
        return;
    }

    std::vector<GLenum> col_bufs;
    for (auto& atta : m_attachments)
    {
        if (atta.dirty) {
            atta.Attach();
        }

        if (atta.tex &&
            atta.type >= AttachmentType::Color0 &&
            atta.type <= AttachmentType::Color15) {
            col_bufs.push_back(TypeConverter::To(atta.type));
        }
    }
    glDrawBuffers(col_bufs.size(), col_bufs.data());
}

//////////////////////////////////////////////////////////////////////////
// class Framebuffer::Attachment
//////////////////////////////////////////////////////////////////////////

void Framebuffer::Attachment::Attach()
{
    if (tex) {
        const GLuint texture = tex ? tex->GetTexID() : 0;
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            TypeConverter::To(type),
            TypeConverter::To(target),
            texture,
            mipmap_level
        );
    }
    if (rbo) {
        rbo->Bind();
    }
    dirty = false;
}

}
}