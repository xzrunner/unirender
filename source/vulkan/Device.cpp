#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/VertexArray.h"
#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/Texture.h"

#include <SM_Vector.h>

#include <iostream>

#include <assert.h>

namespace ur
{
namespace vulkan
{

Device::Device()
{
    m_info.Init();
}

std::shared_ptr<ur::VertexArray>
Device::GetVertexArray(PrimitiveType prim, VertexLayoutType layout) const
{
    return nullptr;
}

std::shared_ptr<ur::VertexArray>
Device::CreateVertexArray() const
{
	return std::make_shared<ur::vulkan::VertexArray>(*this);
}

std::shared_ptr<ur::Framebuffer>
Device::CreateFramebuffer() const
{
    return nullptr;
}

std::shared_ptr<ur::RenderBuffer>
Device::CreateRenderBuffer(int width, int height, InternalFormat format, AttachmentType attach) const
{
    return nullptr;
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::string& vs, const std::string& fs, const std::string& gs,
                            const std::vector<std::string>& attr_names) const
{
	return std::make_shared<ur::vulkan::ShaderProgram>(m_info.device, vs, fs, gs, attr_names);
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::string& cs) const
{
    return nullptr;
}

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
	return std::make_shared<ur::vulkan::VertexBuffer>();
}

std::shared_ptr<ur::IndexBuffer>
Device::CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
	return std::make_shared<ur::vulkan::IndexBuffer>(usage_hint, size_in_bytes);
}

std::shared_ptr<ur::WritePixelBuffer>
Device::CreateWritePixelBuffer(BufferUsageHint hint, int size_in_bytes) const
{
    return nullptr;
}

std::shared_ptr<ur::ComputeBuffer>
Device::CreateComputeBuffer(const std::vector<int>& buf, size_t index) const
{
    return nullptr;
}

std::shared_ptr<ur::ComputeBuffer>
Device::CreateComputeBuffer(const std::vector<float>& buf, size_t index) const
{
    return nullptr;
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(const TextureDescription& desc, const void* pixels) const
{
	return std::make_shared<ur::vulkan::Texture>(desc, *this);
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(size_t width, size_t height, TextureFormat format, const void* buf, size_t buf_sz) const
{
	TextureDescription desc;
	return std::make_shared<ur::vulkan::Texture>(desc, *this);
}

std::shared_ptr<ur::TextureSampler>
Device::CreateTextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter, TextureWrap wrap_s, TextureWrap wrap_t) const
{
    return nullptr;
}

std::shared_ptr<ur::Texture>
Device::CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const
{
	return nullptr;
}

std::shared_ptr<ur::TextureSampler>
Device::GetTextureSampler(TextureSamplerType type) const
{
    return nullptr;
}

void Device::DispatchCompute(int thread_group_count) const
{
}

void Device::ReadPixels(const unsigned char* pixels, ur::TextureFormat format,
                        int x, int y, int w, int h) const
{
}

void Device::ReadPixels(const short* pixels, ur::TextureFormat format,
                        int x, int y, int w, int h) const
{
}

}
}