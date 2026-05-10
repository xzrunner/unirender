#pragma once

#include "unirender/VertexBuffer.h"

#include <memory>
#include <vector>

namespace ur
{
namespace metal
{

class VertexBuffer : public ur::VertexBuffer
{
public:
    VertexBuffer(void* mtl_device, int size_in_bytes);
    virtual ~VertexBuffer();

    virtual int GetSizeInBytes() const override { return m_size_in_bytes; }
    virtual BufferUsageHint GetUsageHint() const override { return m_usage_hint; }

    virtual void ReadFromMemory(const void* data, int size, int offset) override;
    virtual void* WriteToMemory(int size, int offset) override;

    virtual void Bind() const override {}
    static void UnBind() {}

    virtual void Reserve(int size_in_bytes) override;

    virtual size_t GetVertexCount() const override { return m_vertex_count; }
    void SetVertexCount(size_t count) { m_vertex_count = count; }

    void* GetMTLBuffer() const { return m_mtl_buffer; }

private:
    void CreateBuffer(int size);

    void* m_mtl_device = nullptr;   // id<MTLDevice>
    void* m_mtl_buffer = nullptr;   // id<MTLBuffer>

    int m_size_in_bytes = 0;
    size_t m_vertex_count = 0;
    BufferUsageHint m_usage_hint = BufferUsageHint::StaticDraw;

}; // VertexBuffer

}
}
