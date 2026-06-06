#import <Metal/Metal.h>
#include "unirender/metal/VertexBuffer.h"
#include <cstring>
#include <cassert>

namespace ur
{
namespace metal
{

VertexBuffer::VertexBuffer(void* mtl_device, int size_in_bytes)
    : m_mtl_device(mtl_device)
    , m_size_in_bytes(size_in_bytes)
{
    if (size_in_bytes > 0) {
        CreateBuffer(size_in_bytes);
    }
}

VertexBuffer::~VertexBuffer()
{
    if (m_mtl_buffer) {
        CFRelease(m_mtl_buffer);
        m_mtl_buffer = nullptr;
    }
}

void VertexBuffer::CreateBuffer(int size)
{
    if (m_mtl_buffer) {
        CFRelease(m_mtl_buffer);
        m_mtl_buffer = nullptr;
    }
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_mtl_device;
    id<MTLBuffer> buf = [device newBufferWithLength:size
                                           options:MTLResourceStorageModeShared];
    m_mtl_buffer = (__bridge_retained void*)buf;
    m_size_in_bytes = size;
}

void VertexBuffer::ReadFromMemory(const void* data, int size, int offset)
{
    if (!data || size <= 0) return;

    // Metal defers a draw until the command buffer commits, but this buffer is
    // reused across many 2D batches per frame (SpriteRenderer flushes on every
    // texture/state change). Reusing one MTLBuffer would make every encoded-but-
    // not-yet-executed draw read the LAST batch's data -- dropped glyphs and stray
    // triangles linking unrelated primitives. On a full rewrite (offset 0) orphan
    // the buffer (allocate a fresh one) so each pending draw keeps its own snapshot;
    // Metal retains the old buffer until its command buffer completes, then frees it.
    if (offset == 0) {
        CreateBuffer(size);
    } else if (!m_mtl_buffer || offset + size > m_size_in_bytes) {
        CreateBuffer(offset + size);
    }

    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_mtl_buffer;
    memcpy((uint8_t*)[buf contents] + offset, data, size);
}

void* VertexBuffer::WriteToMemory(int size, int offset)
{
    if (!m_mtl_buffer) return nullptr;

    void* dst = new uint8_t[size];
    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_mtl_buffer;
    memcpy(dst, (uint8_t*)[buf contents] + offset, size);
    return dst;
}

void VertexBuffer::Reserve(int size_in_bytes)
{
    if (size_in_bytes > m_size_in_bytes) {
        CreateBuffer(size_in_bytes);
    }
}

}
}
