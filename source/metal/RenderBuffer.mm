#import <Metal/Metal.h>
#include "unirender/metal/RenderBuffer.h"
#include <cassert>

namespace
{

MTLPixelFormat ToMTLFormat(ur::InternalFormat fmt)
{
    switch (fmt)
    {
    case ur::InternalFormat::RGBA8:       return MTLPixelFormatRGBA8Unorm;
    case ur::InternalFormat::RGBA16F:     return MTLPixelFormatRGBA16Float;
    case ur::InternalFormat::RGBA32F:     return MTLPixelFormatRGBA32Float;
    case ur::InternalFormat::RED:         return MTLPixelFormatR8Unorm;
    case ur::InternalFormat::R16:         return MTLPixelFormatR16Unorm;
    case ur::InternalFormat::R16F:        return MTLPixelFormatR16Float;
    case ur::InternalFormat::RG16F:       return MTLPixelFormatRG16Float;
    case ur::InternalFormat::DepthComponent: return MTLPixelFormatDepth32Float;
    case ur::InternalFormat::DepthStencil:   return MTLPixelFormatDepth32Float_Stencil8;
    default:                              return MTLPixelFormatRGBA8Unorm;
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
