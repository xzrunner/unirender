#pragma once

#include "unirender/BufferUsageHint.h"

namespace ur
{

enum class IndexBufferDataType
{
    UnsignedShort,
    UnsignedInt
};

class IndexBuffer
{
public:
    virtual ~IndexBuffer() {}

    virtual int GetSizeInBytes() const = 0;
    virtual BufferUsageHint GetUsageHint() const = 0;
    virtual IndexBufferDataType GetDataType() const = 0;

    virtual void ReadFromMemory(const void* data, int size, int offset) = 0;
    virtual void* WriteToMemory(int size, int offset) = 0;

    virtual void Bind() const = 0;

    virtual void Reset(int size_in_bytes) = 0;

}; // IndexBuffer

}