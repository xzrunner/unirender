#pragma once

#include "unirender/ReadPixelBuffer.h"
#include "unirender/opengl/PixelBuffer.h"

namespace ur
{
namespace opengl
{

class ReadPixelBuffer : public ur::ReadPixelBuffer
{
public:
    ReadPixelBuffer(BufferUsageHint hint, int size_in_bytes);

    virtual int GetSizeInBytes() const override;
    virtual BufferUsageHint GetUsageHint() const override;

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    static void UnBind();

private:
    PixelBuffer m_buf;

}; // ReadPixelBuffer

}
}