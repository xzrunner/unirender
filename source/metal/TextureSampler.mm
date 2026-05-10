#import <Metal/Metal.h>
#include "unirender/metal/TextureSampler.h"

namespace
{

MTLSamplerMinMagFilter ToMinMag(ur::TextureMinificationFilter f)
{
    switch (f) {
    case ur::TextureMinificationFilter::Nearest:
    case ur::TextureMinificationFilter::NearestMipmapNearest:
    case ur::TextureMinificationFilter::NearestMipmapLinear:
        return MTLSamplerMinMagFilterNearest;
    default:
        return MTLSamplerMinMagFilterLinear;
    }
}

MTLSamplerMinMagFilter ToMinMag(ur::TextureMagnificationFilter f)
{
    return (f == ur::TextureMagnificationFilter::Nearest)
        ? MTLSamplerMinMagFilterNearest
        : MTLSamplerMinMagFilterLinear;
}

MTLSamplerMipFilter ToMip(ur::TextureMinificationFilter f)
{
    switch (f) {
    case ur::TextureMinificationFilter::NearestMipmapNearest:
    case ur::TextureMinificationFilter::LinearMipmapNearest:
        return MTLSamplerMipFilterNearest;
    case ur::TextureMinificationFilter::NearestMipmapLinear:
    case ur::TextureMinificationFilter::LinearMipmapLinear:
        return MTLSamplerMipFilterLinear;
    default:
        return MTLSamplerMipFilterNotMipmapped;
    }
}

MTLSamplerAddressMode ToWrap(ur::TextureWrap w)
{
    switch (w) {
    case ur::TextureWrap::ClampToEdge:   return MTLSamplerAddressModeClampToEdge;
    case ur::TextureWrap::ClampToBorder: return MTLSamplerAddressModeClampToBorderColor;
    case ur::TextureWrap::Repeat:        return MTLSamplerAddressModeRepeat;
    case ur::TextureWrap::MirroredRepeat:return MTLSamplerAddressModeMirrorRepeat;
    default:                             return MTLSamplerAddressModeClampToEdge;
    }
}

}

namespace ur
{
namespace metal
{

TextureSampler::TextureSampler(void* mtl_device,
                               TextureMinificationFilter min_filter,
                               TextureMagnificationFilter mag_filter,
                               TextureWrap wrap_s, TextureWrap wrap_t,
                               float max_anisotropy)
    : m_min_filter(min_filter)
    , m_mag_filter(mag_filter)
    , m_wrap_s(wrap_s)
    , m_wrap_t(wrap_t)
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)mtl_device;

    MTLSamplerDescriptor* sd = [[MTLSamplerDescriptor alloc] init];
    sd.minFilter    = ToMinMag(min_filter);
    sd.magFilter    = ToMinMag(mag_filter);
    sd.mipFilter    = ToMip(min_filter);
    sd.sAddressMode = ToWrap(wrap_s);
    sd.tAddressMode = ToWrap(wrap_t);
    sd.maxAnisotropy = static_cast<NSUInteger>(max_anisotropy);

    id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:sd];
    m_mtl_sampler = (__bridge_retained void*)sampler;
}

TextureSampler::~TextureSampler()
{
    if (m_mtl_sampler) {
        CFRelease(m_mtl_sampler);
        m_mtl_sampler = nullptr;
    }
}

}
}
