#pragma once

#include "unirender/IndexBuffer.h"
#include "unirender/opengl/Buffer.h"

namespace ur
{
namespace opengl
{

class IndexBuffer : public ur::IndexBuffer
{
public:
    IndexBuffer(BufferUsageHint usage_hint, int size_in_bytes);

    virtual int GetSizeInBytes() const override;
    virtual BufferUsageHint GetUsageHint() const override;
    virtual IndexBufferDataType GetDataType() const override;

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override;
    static void UnBind();

    virtual void Reset(int size_in_bytes) override;

    virtual void SetDataType(IndexBufferDataType data_type) override {
        m_data_type = data_type;
    }

private:
    Buffer m_buf;

    IndexBufferDataType m_data_type = IndexBufferDataType::UnsignedShort;

}; // IndexBuffer

}
}