#pragma once

#include "unirender/StorageBuffer.h"
#include "unirender/opengl/Buffer.h"

namespace ur
{
namespace opengl
{

class StorageBuffer : public ur::StorageBuffer
{
public:
	StorageBuffer(BufferUsageHint usage_hint, int size_in_bytes);

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    virtual void UnBind() const override;

    void BindIndex(int idx);

private:
	Buffer m_buf;

}; // StorageBuffer

}
}