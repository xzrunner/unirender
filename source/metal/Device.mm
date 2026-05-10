#import <Metal/Metal.h>

#include "unirender/metal/Device.h"
#include "unirender/metal/VertexArray.h"
#include "unirender/metal/VertexBuffer.h"
#include "unirender/metal/IndexBuffer.h"
#include "unirender/metal/Texture.h"
#include "unirender/metal/TextureSampler.h"
#include "unirender/metal/ShaderProgram.h"
#include "unirender/metal/Framebuffer.h"
#include "unirender/metal/RenderBuffer.h"
#include "unirender/metal/UniformBuffer.h"
#include "unirender/TextureDescription.h"

#include <iostream>
#include <cassert>

namespace ur
{
namespace metal
{

Device::Device(std::ostream& logger)
{
    Init();
}

Device::~Device()
{
    if (m_cmd_queue) { CFRelease(m_cmd_queue); m_cmd_queue = nullptr; }
    if (m_mtl_device) { CFRelease(m_mtl_device); m_mtl_device = nullptr; }
}

void Device::Init()
{
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    assert(device && "Metal is not supported on this system");
    m_mtl_device = (__bridge_retained void*)device;

    id<MTLCommandQueue> queue = [device newCommandQueue];
    assert(queue);
    m_cmd_queue = (__bridge_retained void*)queue;

    m_max_num_vert_attrs        = 31;
    m_max_num_tex_units         = 31;
    m_max_num_color_attachments = 8;
    m_max_num_img_units         = 8;
}

// ---------------------------------------------------------------------------
// VertexArray
// ---------------------------------------------------------------------------

std::shared_ptr<ur::VertexArray>
Device::GetVertexArray(PrimitiveType prim, VertexLayoutType layout, bool unit) const
{
    // TODO: cached quad / cube vertex arrays
    return nullptr;
}

std::shared_ptr<ur::VertexArray>
Device::CreateVertexArray() const
{
    return std::make_shared<metal::VertexArray>();
}

// ---------------------------------------------------------------------------
// Framebuffer / RenderBuffer
// ---------------------------------------------------------------------------

std::shared_ptr<ur::Framebuffer>
Device::CreateFramebuffer() const
{
    return std::make_shared<metal::Framebuffer>(m_mtl_device);
}

std::shared_ptr<ur::RenderBuffer>
Device::CreateRenderBuffer(int width, int height, InternalFormat format) const
{
    return std::make_shared<metal::RenderBuffer>(m_mtl_device, width, height, format);
}

// ---------------------------------------------------------------------------
// ShaderProgram
// ---------------------------------------------------------------------------

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::vector<unsigned int>& vs,
                            const std::vector<unsigned int>& fs,
                            const std::vector<unsigned int>& tcs,
                            const std::vector<unsigned int>& tes,
                            const std::vector<unsigned int>& gs) const
{
    if (vs.empty() || fs.empty()) return nullptr;
    return std::make_shared<metal::ShaderProgram>(m_mtl_device, vs, fs);
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::vector<unsigned int>& cs) const
{
    return nullptr; // TODO: compute
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::string& cs) const
{
    if (cs.empty()) return nullptr;
    return std::make_shared<metal::ShaderProgram>(m_mtl_device, cs);
}

// ---------------------------------------------------------------------------
// Buffers
// ---------------------------------------------------------------------------

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
    return std::make_shared<metal::VertexBuffer>(m_mtl_device, size_in_bytes);
}

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(const void* data, size_t size) const
{
    auto vb = std::make_shared<metal::VertexBuffer>(m_mtl_device, static_cast<int>(size));
    if (data && size > 0) {
        vb->ReadFromMemory(data, static_cast<int>(size), 0);
    }
    return vb;
}

std::shared_ptr<ur::IndexBuffer>
Device::CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
    return std::make_shared<metal::IndexBuffer>(m_mtl_device, size_in_bytes);
}

std::shared_ptr<ur::WritePixelBuffer>
Device::CreateWritePixelBuffer(BufferUsageHint hint, int size_in_bytes) const
{
    return nullptr; // Metal doesn't need PBO; use MTLBuffer directly
}

std::shared_ptr<ur::ComputeBuffer>
Device::CreateComputeBuffer(const void* data, size_t size, size_t index) const
{
    return nullptr; // TODO
}

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------

std::shared_ptr<ur::Texture>
Device::CreateTexture(const TextureDescription& desc, const void* pixels) const
{
    auto tex = std::make_shared<metal::Texture>(m_mtl_device, desc);
    if (pixels) {
        tex->ReadFromMemory(pixels, desc.format, desc.width, desc.height, desc.depth, 1);
    }
    return tex;
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(size_t width, size_t height, TextureFormat format,
                      const void* buf, size_t buf_sz, bool gamma_correction) const
{
    TextureDescription desc;
    desc.target = TextureTarget::Texture2D;
    desc.width  = static_cast<int>(width);
    desc.height = static_cast<int>(height);
    desc.format = format;
    desc.gamma_correction = gamma_correction;
    return CreateTexture(desc, buf);
}

std::shared_ptr<ur::Texture>
Device::CreateTexture3D(size_t width, size_t height, size_t depth,
                        ur::TextureFormat format, const void* buf, size_t buf_sz,
                        bool gamma_correction) const
{
    TextureDescription desc;
    desc.target = TextureTarget::Texture3D;
    desc.width  = static_cast<int>(width);
    desc.height = static_cast<int>(height);
    desc.depth  = static_cast<int>(depth);
    desc.format = format;
    desc.gamma_correction = gamma_correction;
    return CreateTexture(desc, buf);
}

std::shared_ptr<ur::Texture>
Device::CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const
{
    // TODO: create cube map from 6 face textures
    return nullptr;
}

std::shared_ptr<ur::TextureSampler>
Device::CreateTextureSampler(TextureMinificationFilter min_filter,
                             TextureMagnificationFilter mag_filter,
                             TextureWrap wrap_s, TextureWrap wrap_t,
                             float max_anistropy) const
{
    return std::make_shared<metal::TextureSampler>(
        m_mtl_device, min_filter, mag_filter, wrap_s, wrap_t, max_anistropy);
}

// ---------------------------------------------------------------------------
// UniformBuffer
// ---------------------------------------------------------------------------

std::shared_ptr<ur::UniformBuffer>
Device::CreateUniformBuffer(const void* data, size_t size) const
{
    return std::make_shared<metal::UniformBuffer>(m_mtl_device, data, size);
}

// ---------------------------------------------------------------------------
// Compute dispatch
// ---------------------------------------------------------------------------

void Device::DispatchCompute(int num_groups_x, int num_groups_y, int num_groups_z) const
{
    // TODO: MTLComputeCommandEncoder
}

// ---------------------------------------------------------------------------
// ReadPixels
// ---------------------------------------------------------------------------

void Device::ReadPixels(const unsigned char* pixels, ur::TextureFormat fmt,
                        int x, int y, int w, int h) const
{
    // TODO: blit + readback
}

void Device::ReadPixels(const short* pixels, ur::TextureFormat fmt,
                        int x, int y, int w, int h) const
{
    // TODO
}

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------

void Device::PushDebugGroup(const std::string& msg) const {}
void Device::PopDebugGroup() const {}

}
}
