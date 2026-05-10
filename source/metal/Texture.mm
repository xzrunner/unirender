#import <Metal/Metal.h>
#include "unirender/metal/Texture.h"
#include "unirender/TextureUtility.h"
#include <cstring>
#include <cassert>

namespace
{

MTLPixelFormat ToMTLPixelFormat(ur::TextureFormat fmt, bool gamma)
{
    switch (fmt)
    {
    case ur::TextureFormat::RGBA8:    return gamma ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm;
    case ur::TextureFormat::BGRA8:    return gamma ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
    case ur::TextureFormat::RGB:      return MTLPixelFormatRGBA8Unorm; // Metal has no RGB8; promote to RGBA
    case ur::TextureFormat::RGBA4:    return MTLPixelFormatABGR4Unorm;
    case ur::TextureFormat::RGB565:   return MTLPixelFormatB5G6R5Unorm;
    case ur::TextureFormat::A8:       return MTLPixelFormatA8Unorm;
    case ur::TextureFormat::RED:      return MTLPixelFormatR8Unorm;
    case ur::TextureFormat::R16:      return MTLPixelFormatR16Unorm;
    case ur::TextureFormat::RGBA16F:  return MTLPixelFormatRGBA16Float;
    case ur::TextureFormat::RGB16F:   return MTLPixelFormatRGBA16Float; // promote
    case ur::TextureFormat::RG16F:    return MTLPixelFormatRG16Float;
    case ur::TextureFormat::R16F:     return MTLPixelFormatR16Float;
    case ur::TextureFormat::RGBA32F:  return MTLPixelFormatRGBA32Float;
    case ur::TextureFormat::RGB32F:   return MTLPixelFormatRGBA32Float; // promote
    case ur::TextureFormat::RG32F:    return MTLPixelFormatRG32Float;
    case ur::TextureFormat::R32F:     return MTLPixelFormatR32Float;
    case ur::TextureFormat::DEPTH:    return MTLPixelFormatDepth32Float;
    case ur::TextureFormat::DEPTH24STENCIL8: return MTLPixelFormatDepth32Float_Stencil8;
    default:                          return MTLPixelFormatRGBA8Unorm;
    }
}

size_t BytesPerPixel(ur::TextureFormat fmt)
{
    switch (fmt)
    {
    case ur::TextureFormat::A8:
    case ur::TextureFormat::RED:      return 1;
    case ur::TextureFormat::R16:
    case ur::TextureFormat::R16F:
    case ur::TextureFormat::RGBA4:
    case ur::TextureFormat::RGB565:   return 2;
    case ur::TextureFormat::RGB:      return 3;
    case ur::TextureFormat::RGBA8:
    case ur::TextureFormat::BGRA8:
    case ur::TextureFormat::RG16F:
    case ur::TextureFormat::R32F:
    case ur::TextureFormat::DEPTH:    return 4;
    case ur::TextureFormat::RGB16F:
    case ur::TextureFormat::RGBA16F:  return 8;
    case ur::TextureFormat::RGB32F:   return 12;
    case ur::TextureFormat::RGBA32F:
    case ur::TextureFormat::RG32F:    return 8;
    default:                          return 4;
    }
}

}

namespace ur
{
namespace metal
{

Texture::Texture(void* mtl_device, const TextureDescription& desc)
    : m_mtl_device(mtl_device)
    , m_desc(desc)
{
    if (desc.width > 0 && desc.height > 0) {
        CreateMTLTexture();
    }
}

Texture::~Texture()
{
    if (m_mtl_texture) {
        CFRelease(m_mtl_texture);
        m_mtl_texture = nullptr;
    }
}

void Texture::CreateMTLTexture()
{
    if (m_mtl_texture) {
        CFRelease(m_mtl_texture);
        m_mtl_texture = nullptr;
    }

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_mtl_device;
    MTLTextureDescriptor* td = nil;

    MTLPixelFormat pf = ToMTLPixelFormat(m_desc.format, m_desc.gamma_correction);

    switch (m_desc.target)
    {
    case TextureTarget::Texture2D:
    default:
        td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pf
                                                               width:m_desc.width
                                                              height:m_desc.height
                                                           mipmapped:m_desc.gen_mipmaps ? YES : NO];
        td.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
        break;
    case TextureTarget::Texture3D:
        td = [[MTLTextureDescriptor alloc] init];
        td.textureType = MTLTextureType3D;
        td.pixelFormat = pf;
        td.width  = m_desc.width;
        td.height = m_desc.height;
        td.depth  = m_desc.depth > 0 ? m_desc.depth : 1;
        td.usage  = MTLTextureUsageShaderRead;
        break;
    case TextureTarget::TextureCubeMap:
        td = [[MTLTextureDescriptor alloc] init];
        td.textureType = MTLTextureTypeCube;
        td.pixelFormat = pf;
        td.width  = m_desc.width;
        td.height = m_desc.height;
        td.usage  = MTLTextureUsageShaderRead;
        break;
    }

    td.storageMode = MTLStorageModeShared;
    id<MTLTexture> tex = [device newTextureWithDescriptor:td];
    m_mtl_texture = (__bridge_retained void*)tex;
}

void Texture::Upload(const void* pixels, int x, int y, int w, int h,
                     int miplevel, int row_alignment)
{
    if (!pixels || !m_mtl_texture) return;

    id<MTLTexture> tex = (__bridge id<MTLTexture>)m_mtl_texture;
    size_t bpp = BytesPerPixel(m_desc.format);
    // Metal has no RGB8; if source is RGB we need to expand to RGBA
    bool needs_rgb_expand = (m_desc.format == TextureFormat::RGB);
    size_t src_bpp = needs_rgb_expand ? 3 : bpp;
    size_t dst_bpp = needs_rgb_expand ? 4 : bpp;
    size_t bytesPerRow = dst_bpp * w;

    if (needs_rgb_expand) {
        // Expand RGB -> RGBA
        size_t pixel_count = w * h;
        uint8_t* rgba = new uint8_t[pixel_count * 4];
        const uint8_t* src = static_cast<const uint8_t*>(pixels);
        for (size_t i = 0; i < pixel_count; ++i) {
            rgba[i*4+0] = src[i*3+0];
            rgba[i*4+1] = src[i*3+1];
            rgba[i*4+2] = src[i*3+2];
            rgba[i*4+3] = 255;
        }
        MTLRegion region = MTLRegionMake2D(x, y, w, h);
        [tex replaceRegion:region mipmapLevel:miplevel withBytes:rgba bytesPerRow:bytesPerRow];
        delete[] rgba;
    } else {
        MTLRegion region = MTLRegionMake2D(x, y, w, h);
        [tex replaceRegion:region mipmapLevel:miplevel withBytes:pixels bytesPerRow:bytesPerRow];
    }
}

void Texture::ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler)
{
    // Metal uses MTLSamplerState objects, bound via encoder, not on the texture itself.
    // Stored for use at draw time by Context.
}

void Texture::ReadFromMemory(const void* pixels, TextureFormat fmt,
                             int width, int height, int depth, int row_alignment)
{
    m_desc.format = fmt;
    m_desc.width  = width;
    m_desc.height = height;
    m_desc.depth  = depth;
    CreateMTLTexture();

    if (pixels) {
        Upload(pixels, 0, 0, width, height, 0, row_alignment);
    }
}

void* Texture::WriteToMemory(int size) const
{
    void* data = new uint8_t[size];
    memset(data, 0, size);
    WriteToMemory(data);
    return data;
}

void Texture::WriteToMemory(void* data) const
{
    if (!m_mtl_texture || !data) return;

    id<MTLTexture> tex = (__bridge id<MTLTexture>)m_mtl_texture;
    size_t bpp = BytesPerPixel(m_desc.format);
    MTLRegion region = MTLRegionMake2D(0, 0, m_desc.width, m_desc.height);
    [tex getBytes:data bytesPerRow:bpp * m_desc.width fromRegion:region mipmapLevel:0];
}

}
}
