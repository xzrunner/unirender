#pragma once

#include "unirender/IndexBuffer.h"

namespace ur
{
namespace metal
{

class IndexBuffer : public ur::IndexBuffer
{
public:
    IndexBuffer(void* mtl_device, int size_in_bytes);
    virtual ~IndexBuffer();

    virtual BufferUsageHint GetUsageHint() const override { return m_usage_hint; }
    virtual IndexBufferDataType GetDataType() const override { return m_data_type; }

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override {}
    static void UnBind() {}

    virtual void Reserve(int size_in_bytes) override;
    virtual void SetDataType(IndexBufferDataType data_type) override { m_data_type = data_type; }

    void* GetMTLBuffer() const { return m_mtl_buffer; }
    size_t GetCount() const { return m_count; }

private:
    void CreateBuffer(int size);

    void* m_mtl_device = nullptr;
    void* m_mtl_buffer = nullptr;

    int m_size_in_bytes = 0;
    size_t m_count = 0;
    BufferUsageHint m_usage_hint = BufferUsageHint::StaticDraw;
    IndexBufferDataType m_data_type = IndexBufferDataType::UnsignedShort;

}; // IndexBuffer

}
}
