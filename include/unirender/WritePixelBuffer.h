#pragma once

#include "unirender/BufferUsageHint.h"

namespace ur
{

class WritePixelBuffer
{
public:
    virtual ~WritePixelBuffer() {}

    virtual int GetSizeInBytes() const = 0;
    virtual BufferUsageHint GetUsageHint() const = 0;

    virtual void ReadFromMemory(const void* data, int size, int offset) = 0;
    virtual void* WriteToMemory(int size, int offset) = 0;

    virtual void Bind() const = 0;
    virtual void UnBind() const = 0;

    virtual unsigned char* Map() const = 0;
    virtual void Unmap() const = 0;

}; // WritePixelBuffer

}