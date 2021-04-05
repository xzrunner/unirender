#pragma once

#include "unirender/ComputeBuffer.h"

#include <unirender/opengl/opengl.h>

namespace ur
{
namespace opengl
{

class ComputeBuffer : public ur::ComputeBuffer
{
public:
    ComputeBuffer(const void* data, size_t size, size_t index)
    {
        glGenBuffers(1, &m_id);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STREAM_COPY);
    }
    virtual ~ComputeBuffer();

    virtual void GetComputeBufferData(void* data, size_t size) const override;

private:
    GLuint m_id = 0;

}; // ComputeBuffer

}
}