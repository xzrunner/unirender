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

    virtual BufferUsageHint GetUsageHint() const = 0;
    virtual IndexBufferDataType GetDataType() const = 0;

    virtual void ReadFromMemory(const void* data, int size, int offset) = 0;
    virtual void* WriteToMemory(int size, int offset) = 0;

    virtual void Bind() const = 0;

    virtual void Reserve(int size_in_bytes) = 0;

    virtual void SetDataType(IndexBufferDataType data_type) = 0;

    int GetCount() const { return m_count; }
    void SetCount(int count) { m_count = count; }

private:
    int m_count = 0;

}; // IndexBuffer

}