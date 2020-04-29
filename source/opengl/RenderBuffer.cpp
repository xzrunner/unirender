#include "unirender/opengl/RenderBuffer.h"
#include "unirender/opengl/TypeConverter.h"

namespace ur
{
namespace opengl
{

RenderBuffer::RenderBuffer(int width, int height,
                           InternalFormat format, AttachmentType attach)
    : m_attach(attach)
{
    glGenRenderbuffers(1, &m_id);
    glBindRenderbuffer(GL_RENDERBUFFER, m_id);
    glRenderbufferStorage(GL_RENDERBUFFER, TypeConverter::To(format), width, height);
}

RenderBuffer::~RenderBuffer()
{
    glDeleteRenderbuffers(1, &m_id);
}

void RenderBuffer::Bind() const
{
    auto attach = TypeConverter::To(m_attach);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach, GL_RENDERBUFFER, m_id);
}

}
}