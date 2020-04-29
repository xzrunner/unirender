#include "unirender/opengl/WritePixelBuffer.h"
#include "unirender/opengl/TypeConverter.h"

namespace ur
{
namespace opengl
{

WritePixelBuffer::WritePixelBuffer(BufferUsageHint hint, int size_in_bytes)
    : m_buf(BufferTarget::PixelUnpackBuffer, hint, size_in_bytes)
{
}

int WritePixelBuffer::GetSizeInBytes() const
{
    return m_buf.GetSizeInBytes();
}

BufferUsageHint WritePixelBuffer::GetUsageHint() const
{
    return m_buf.GetUsageHint();
}

void WritePixelBuffer::ReadFromMemory(const void* data, int size, int offset)
{
    m_buf.ReadFromMemory(data, size, offset);
}

void* WritePixelBuffer::WriteToMemory(int size, int offset)
{
    return m_buf.WriteToMemory(size, offset);
}

void WritePixelBuffer::Bind() const
{
    m_buf.Bind();
}

void WritePixelBuffer::UnBind()
{
    const auto target = TypeConverter::To(BufferTarget::PixelUnpackBuffer);
    glBindBuffer(target, 0);
}

unsigned char* WritePixelBuffer::Map() const
{
    if (!m_mapped_ptr) {
        Bind();
        m_mapped_ptr = reinterpret_cast<unsigned char*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
    }
    return m_mapped_ptr;
}

void WritePixelBuffer::Unmap() const
{
    if (m_mapped_ptr) {
        Bind();
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        m_mapped_ptr = nullptr;
    }
}

}
}