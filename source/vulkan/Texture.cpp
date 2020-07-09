#include "unirender/vulkan/Texture.h"

namespace ur
{
namespace vulkan
{

Texture::Texture(TextureDescription desc, const ur::Device& device)
{

}

Texture::~Texture()
{

}

void Texture::Bind() const
{

}

void Texture::UnBind(TextureTarget target)
{

}

void Texture::Upload(const void* pixels, int x, int y, int w, int h, int miplevel, int row_alignment)
{

}

void Texture::ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler)
{

}

}
}