#pragma once

#include "unirender/InternalFormat.h"
#include "unirender/RenderBuffer.h"
#include "unirender/opengl/opengl.h"

namespace ur
{
namespace opengl
{

class RenderBuffer : public ur::RenderBuffer
{
public:
    RenderBuffer(int width, int height, InternalFormat format);
    virtual ~RenderBuffer();

    virtual void Bind(AttachmentType attach) const override;

private:
    GLuint m_id = 0;

}; // RenderBuffer
}
}