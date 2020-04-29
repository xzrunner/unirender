#pragma once

#include "unirender/VertexBuffer.h"
#include "unirender/opengl/Buffer.h"

namespace ur
{
namespace opengl
{

class VertexBuffer : public ur::VertexBuffer
{
public:
    VertexBuffer(BufferUsageHint usage_hint, int size_in_bytes);

    virtual int GetSizeInBytes() const override;
    virtual BufferUsageHint GetUsageHint() const override;

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    static void UnBind();

    virtual void Reset(int size_in_bytes) override;

private:
    Buffer m_buf;

}; // VertexBuffer

}
}