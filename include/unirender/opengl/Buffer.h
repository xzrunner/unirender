#pragma once

#include "unirender/BufferTarget.h"
#include "unirender/BufferUsageHint.h"
#include "unirender/opengl/opengl.h"

namespace ur
{
namespace opengl
{

class Buffer
{
public:
    Buffer(BufferTarget type, BufferUsageHint hint,
        int size_in_bytes);
    ~Buffer();

    void Bind() const;

    void ReadFromMemory(const void* data, int size, int offset);
    void* WriteToMemory(int size, int offset);

    auto GetSizeInBytes() const { return m_size_in_bytes; }
    auto GetUsageHint() const { return m_usage_hint; }

    void Reset(int size_in_bytes);

private:
    GLuint m_id = 0;

    int m_size_in_bytes = 0;

    BufferTarget    m_type;
    BufferUsageHint m_usage_hint;

}; // Buffer

}
}