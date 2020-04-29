#include "unirender/opengl/PixelBuffer.h"
#include "unirender/opengl/TypeConverter.h"
#include "unirender/Bitmap.h"

#include <assert.h>

namespace ur
{
namespace opengl
{

PixelBuffer::PixelBuffer(BufferTarget type, BufferUsageHint hint,
               int size_in_bytes)
    : m_size_in_bytes(size_in_bytes)
    , m_type(type)
    , m_usage_hint(hint)
{
    assert(size_in_bytes > 0);

    glGenBuffers(1, &m_id);

    Bind();

    glBufferData(TypeConverter::To(m_type), m_size_in_bytes, 0, TypeConverter::To(m_usage_hint));
}

PixelBuffer::~PixelBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void PixelBuffer::Bind() const
{
    glBindBuffer(TypeConverter::To(m_type), m_id);
}

void PixelBuffer::ReadFromMemory(const void* data, int size, int offset)
{
    assert(offset >= 0 && size > 0
        && offset + size <= m_size_in_bytes);

    Bind();

    glBufferSubData(TypeConverter::To(m_type), offset, size, data);
}

void* PixelBuffer::WriteToMemory(int size, int offset) const
{
    assert(offset >= 0 && size > 0
        && offset + size <= m_size_in_bytes);

    void* data = new uint8_t[size];

    Bind();

    glGetBufferSubData(m_id, offset, size, data);

    return data;
}

}
}