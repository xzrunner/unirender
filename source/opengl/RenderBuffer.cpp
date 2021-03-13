#include "unirender/opengl/RenderBuffer.h"
#include "unirender/opengl/TypeConverter.h"

namespace ur
{
namespace opengl
{

RenderBuffer::RenderBuffer(int width, int height, InternalFormat format)
{
    glGenRenderbuffers(1, &m_id);
    glBindRenderbuffer(GL_RENDERBUFFER, m_id);
    glRenderbufferStorage(GL_RENDERBUFFER, TypeConverter::To(format), width, height);
}

RenderBuffer::~RenderBuffer()
{
    glDeleteRenderbuffers(1, &m_id);
}

void RenderBuffer::Bind(AttachmentType attach) const
{
    auto gl_attach = TypeConverter::To(attach);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, gl_attach, GL_RENDERBUFFER, m_id);
}

}
}