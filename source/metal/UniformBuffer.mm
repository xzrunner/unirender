#import <Metal/Metal.h>
#include "unirender/metal/UniformBuffer.h"
#include <cstring>

namespace ur
{
namespace metal
{

UniformBuffer::UniformBuffer(void* mtl_device, const void* data, size_t size)
    : m_mtl_device(mtl_device)
    , m_size(size)
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_mtl_device;
    if (data && size > 0) {
        id<MTLBuffer> buf = [device newBufferWithBytes:data
                                               length:size
                                              options:MTLResourceStorageModeShared];
        m_mtl_buffer = (__bridge_retained void*)buf;
    } else if (size > 0) {
        id<MTLBuffer> buf = [device newBufferWithLength:size
                                               options:MTLResourceStorageModeShared];
        m_mtl_buffer = (__bridge_retained void*)buf;
    }
}

UniformBuffer::~UniformBuffer()
{
    if (m_mtl_buffer) {
        CFRelease(m_mtl_buffer);
        m_mtl_buffer = nullptr;
    }
}

void UniformBuffer::Update(const void* data, size_t size)
{
    if (!data || size == 0) return;

    id<MTLBuffer> buf = (__bridge id<MTLBuffer>)m_mtl_buffer;
    if (!buf || size > m_size) {
        // Recreate
        if (m_mtl_buffer) CFRelease(m_mtl_buffer);
        id<MTLDevice> device = (__bridge id<MTLDevice>)m_mtl_device;
        buf = [device newBufferWithBytes:data length:size
                                options:MTLResourceStorageModeShared];
        m_mtl_buffer = (__bridge_retained void*)buf;
        m_size = size;
    } else {
        memcpy([buf contents], data, size);
    }
}

}
}
