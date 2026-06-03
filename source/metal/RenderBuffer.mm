#import <Metal/Metal.h>
#include "unirender/metal/RenderBuffer.h"
#include <cassert>

namespace
{

MTLPixelFormat ToMTLFormat(ur::InternalFormat fmt)
{
    switch (fmt)
    {
    // ur::InternalFormat was simplified to base formats only (no bit-depth/float
    // variants), so map to 8-bit unorm; HDR formats are not expressible here.
    case ur::InternalFormat::RED:            return MTLPixelFormatR8Unorm;
    case ur::InternalFormat::RG:             return MTLPixelFormatRG8Unorm;
    case ur::InternalFormat::RGB:            return MTLPixelFormatRGBA8Unorm; // Metal has no 24-bit RGB
    case ur::InternalFormat::RGBA:           return MTLPixelFormatRGBA8Unorm;
    case ur::InternalFormat::DepthComponent: return MTLPixelFormatDepth32Float;
    case ur::InternalFormat::DepthStencil:   return MTLPixelFormatDepth32Float_Stencil8;
    default:                                 return MTLPixelFormatRGBA8Unorm;
    }
}

}

namespace ur
{
namespace metal
{

RenderBuffer::RenderBuffer(void* mtl_device, int width, int height, InternalFormat format)
    : m_mtl_device(mtl_device)
    , m_width(width)
    , m_height(height)
    , m_format(format)
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)m_mtl_device;
    MTLTextureDescriptor* td =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:ToMTLFormat(format)
                                                          width:width
                                                         height:height
                                                      mipmapped:NO];
    td.storageMode = MTLStorageModePrivate;
    td.usage       = MTLTextureUsageRenderTarget;

    id<MTLTexture> tex = [device newTextureWithDescriptor:td];
    assert(tex);
    m_mtl_texture = (__bridge_retained void*)tex;
}

RenderBuffer::~RenderBuffer()
{
    if (m_mtl_texture) {
        CFRelease(m_mtl_texture);
        m_mtl_texture = nullptr;
    }
}

}
}
