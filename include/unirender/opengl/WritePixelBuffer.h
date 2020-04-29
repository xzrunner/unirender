#pragma once

#include "unirender/WritePixelBuffer.h"
#include "unirender/opengl/PixelBuffer.h"

namespace ur
{
namespace opengl
{

class WritePixelBuffer : public ur::WritePixelBuffer
{
public:
    WritePixelBuffer(BufferUsageHint hint, int size_in_bytes);

    virtual int GetSizeInBytes() const override;
    virtual BufferUsageHint GetUsageHint() const override;

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    static void UnBind();

    virtual unsigned char* Map() const override;
    virtual void Unmap() const override;

private:
    PixelBuffer m_buf;

    mutable unsigned char* m_mapped_ptr = nullptr;

}; // WritePixelBuffer

}
}