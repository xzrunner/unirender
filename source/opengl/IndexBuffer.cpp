#include "unirender/opengl/IndexBuffer.h"
#include "unirender/opengl/TypeConverter.h"

namespace ur
{
namespace opengl
{

IndexBuffer::IndexBuffer(BufferUsageHint usage_hint, int size_in_bytes)
    : m_buf(BufferTarget::ElementArrayBuffer, usage_hint, size_in_bytes)
{
}

BufferUsageHint IndexBuffer::GetUsageHint() const
{
    return m_buf.GetUsageHint();
}

IndexBufferDataType IndexBuffer::GetDataType() const
{
    return m_data_type;
}

void IndexBuffer::ReadFromMemory(const void* data, int size, int offset)
{
    m_buf.ReadFromMemory(data, size, offset);
}

void* IndexBuffer::WriteToMemory(int size, int offset)
{
    return m_buf.WriteToMemory(size, offset);
}

void IndexBuffer::Bind() const
{
    m_buf.Bind();
}

void IndexBuffer::UnBind()
{
    const auto target = TypeConverter::To(BufferTarget::ElementArrayBuffer);
    glBindBuffer(target, 0);
}

void IndexBuffer::Reserve(int size_in_bytes)
{
    const int old_size = m_buf.GetSizeInBytes();
    if (size_in_bytes > old_size) {
        m_buf.Reset(size_in_bytes);
    }
}

}
}