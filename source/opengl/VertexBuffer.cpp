#include "unirender/opengl/VertexBuffer.h"
#include "unirender/opengl/TypeConverter.h"

namespace ur
{
namespace opengl
{

VertexBuffer::VertexBuffer(BufferUsageHint usage_hint, int size_in_bytes)
    : m_buf(BufferTarget::ArrayBuffer, usage_hint, size_in_bytes)
{
}

int VertexBuffer::GetSizeInBytes() const
{
    return m_buf.GetSizeInBytes();
}

BufferUsageHint VertexBuffer::GetUsageHint() const
{
    return m_buf.GetUsageHint();
}

void VertexBuffer::ReadFromMemory(const void* data, int size, int offset)
{
    m_buf.ReadFromMemory(data, size, offset);
}

void* VertexBuffer::WriteToMemory(int size, int offset)
{
    return m_buf.WriteToMemory(size, offset);
}

void VertexBuffer::Bind() const
{
    m_buf.Bind();
}

void VertexBuffer::UnBind()
{
    const auto target = TypeConverter::To(BufferTarget::ArrayBuffer);
    glBindBuffer(target, 0);
}

void VertexBuffer::Reserve(int size_in_bytes)
{
    const int old_size = m_buf.GetSizeInBytes();
    if (size_in_bytes > old_size) {
        m_buf.Reset(size_in_bytes);
    }
}

}
}