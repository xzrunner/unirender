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

void ComputeBuffer::GetComputeBufferData(std::vector<int>& result) const
{
    glGetNamedBufferSubData(m_id, 0, sizeof(int) * result.size(), result.data());
}

}
}