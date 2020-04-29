#pragma once

#include "unirender/BufferUsageHint.h"

namespace ur
{

class ReadPixelBuffer
{
public:
    virtual ~ReadPixelBuffer() {}

    virtual int GetSizeInBytes() const = 0;
    virtual BufferUsageHint GetUsageHint() const = 0;

    virtual void ReadFromMemory(const void* data, int size, int offset) = 0;
    virtual void* WriteToMemory(int size, int offset) = 0;

    virtual void Bind() const = 0;

}; // ReadPixelBuffer

}