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
    template <typename T>
    ComputeBuffer(const std::vector<T>& buf, size_t index)
    {
        glGenBuffers(1, &m_id);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * buf.size(), buf.data(), GL_STREAM_COPY);
    }
    virtual ~ComputeBuffer();

    virtual void GetComputeBufferData(std::vector<int>& result) const override;

private:
    GLuint m_id = 0;

}; // ComputeBuffer

}
}