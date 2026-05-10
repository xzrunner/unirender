#pragma once

#include "unirender/UniformBuffer.h"

namespace ur
{
namespace metal
{

class UniformBuffer : public ur::UniformBuffer
{
public:
    UniformBuffer(void* mtl_device, const void* data, size_t size);
    virtual ~UniformBuffer();

    virtual void Update(const void* data, size_t size) override;

    void* GetMTLBuffer() const { return m_mtl_buffer; }
    size_t GetSize() const { return m_size; }

private:
    void* m_mtl_device = nullptr;
    void* m_mtl_buffer = nullptr; // id<MTLBuffer>
    size_t m_size = 0;

}; // UniformBuffer

}
}
