#include "unirender/opengl/ComputeBuffer.h"
#include "unirender/opengl/TypeConverter.h"

namespace ur
{
namespace opengl
{

ComputeBuffer::~ComputeBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void ComputeBuffer::GetComputeBufferData(void* data, size_t size) const
{
    glGetNamedBufferSubData(m_id, 0, size, data);
}

}
}