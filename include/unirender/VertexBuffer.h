#pragma once

#include "unirender/BufferUsageHint.h"

namespace ur
{

class VertexBuffer
{
public:
    virtual ~VertexBuffer() {}

    virtual int GetSizeInBytes() const = 0;
    virtual BufferUsageHint GetUsageHint() const = 0;

    virtual void ReadFromMemory(const void* data, int size, int offset) = 0;
    virtual void* WriteToMemory(int size, int offset) = 0;

    virtual void Bind() const = 0;

    virtual void Reserve(int size_in_bytes) = 0;

    virtual size_t GetVertexCount() const = 0;

}; // VertexBuffer

}