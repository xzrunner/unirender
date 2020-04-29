#pragma once

#include "unirender/InternalFormat.h"
#include "unirender/AttachmentType.h"
#include "unirender/RenderBuffer.h"
#include "unirender/opengl/opengl.h"

namespace ur
{
namespace opengl
{

class RenderBuffer : public ur::RenderBuffer
{
public:
    RenderBuffer(int width, int height,
        InternalFormat format, AttachmentType attach);
    virtual ~RenderBuffer();

    virtual void Bind() const override;

private:
    AttachmentType m_attach;

    GLuint m_id = 0;

}; // RenderBuffer

}
}