#pragma once

#include "unirender/BufferTarget.h"
#include "unirender/BufferUsageHint.h"
#include "unirender/opengl/opengl.h"

#include <memory>

namespace ur
{

namespace opengl
{

class PixelBuffer
{
public:
    PixelBuffer(BufferTarget type, BufferUsageHint hint,
        int size_in_bytes);
    ~PixelBuffer();

    void Bind() const;
    void UnBind() const;

    void ReadFromMemory(const void* data, int size, int offset);
    void* WriteToMemory(int size, int offset) const;

    auto GetSizeInBytes() const { return m_size_in_bytes; }
    auto GetUsageHint() const { return m_usage_hint; }

private:
    GLuint m_id = 0;

    int m_size_in_bytes = 0;

    BufferTarget    m_type;
    BufferUsageHint m_usage_hint;

}; // PixelBuffer

}
}