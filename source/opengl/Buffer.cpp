#include "unirender/opengl/Buffer.h"
#include "unirender/opengl/TypeConverter.h"

#include <assert.h>

namespace ur
{
namespace opengl
{

Buffer::Buffer(BufferTarget type, BufferUsageHint hint,
               int size_in_bytes)
    : m_size_in_bytes(size_in_bytes)
    , m_type(type)
    , m_usage_hint(hint)
{
    glGenBuffers(1, &m_id);

    glBindVertexArray(0);
    Bind();

    glBufferData(TypeConverter::To(m_type), m_size_in_bytes, 0, TypeConverter::To(m_usage_hint));
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &m_id);
}

void Buffer::Bind() const
{
    glBindBuffer(TypeConverter::To(m_type), m_id);
}

void Buffer::UnBind() const
{
    glBindBuffer(TypeConverter::To(m_type), 0);
}

void Buffer::ReadFromMemory(const void* data, int size, int offset)
{
    assert(offset >= 0 && size > 0
        && offset + size <= m_size_in_bytes);

    glBindVertexArray(0);
    Bind();

    glBufferSubData(TypeConverter::To(m_type), offset, size, data);
}

void* Buffer::WriteToMemory(int size, int offset)
{
    assert(offset >= 0 && size > 0
        && offset + size <= m_size_in_bytes);

    void* data = new uint8_t[size];

    glBindVertexArray(0);
    Bind();

    glGetBufferSubData(m_id, offset, size, data);

    return data;
}

void Buffer::Reset(int size_in_bytes)
{
    m_size_in_bytes = size_in_bytes;

    glBindVertexArray(0);
    Bind();

    glBufferData(TypeConverter::To(m_type), m_size_in_bytes, 0, TypeConverter::To(m_usage_hint));
}

}
}