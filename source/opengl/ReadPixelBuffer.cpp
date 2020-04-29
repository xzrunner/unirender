#include "unirender/opengl/ReadPixelBuffer.h"
#include "unirender/opengl/TypeConverter.h"

namespace ur
{
namespace opengl
{

ReadPixelBuffer::ReadPixelBuffer(BufferUsageHint hint, int size_in_bytes)
    : m_buf(BufferTarget::PixelPackBuffer, hint, size_in_bytes)
{
}

int ReadPixelBuffer::GetSizeInBytes() const
{
    return m_buf.GetSizeInBytes();
}

BufferUsageHint ReadPixelBuffer::GetUsageHint() const
{
    return m_buf.GetUsageHint();
}

void ReadPixelBuffer::ReadFromMemory(const void* data, int size, int offset)
{
    m_buf.ReadFromMemory(data, size, offset);
}

void* ReadPixelBuffer::WriteToMemory(int size, int offset)
{
    return m_buf.WriteToMemory(size, offset);
}

void ReadPixelBuffer::Bind() const
{
    m_buf.Bind();
}

void ReadPixelBuffer::UnBind()
{
    const auto target = TypeConverter::To(BufferTarget::PixelPackBuffer);
    glBindBuffer(target, 0);
}

}
}