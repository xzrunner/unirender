#include "unirender/opengl/ComputeBuffer.h"
#include "unirender/opengl/TypeConverter.h"

namespace ur
{
namespace opengl
{

//template <typename T>
//ComputeBuffer::ComputeBuffer(const std::vector<T>& buf, size_t index)
//{
//    glGenBuffers(1, &m_id);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_id);
//    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(buf), &buf.front(), GL_STREAM_COPY);
//}

ComputeBuffer::~ComputeBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void ComputeBuffer::GetComputeBufferData(std::vector<int>& result) const
{
    glGetNamedBufferSubData(m_id, 0, sizeof(result), result.data());
}

}
}