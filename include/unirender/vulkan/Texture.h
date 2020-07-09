#pragma once

#include "unirender/Texture.h"
#include "unirender/TextureDescription.h"
#include "unirender/TextureTarget.h"

namespace ur
{

class Device;

namespace vulkan
{

class Texture : public ur::Texture
{
public:
    Texture(TextureDescription desc, const ur::Device& device);
    virtual ~Texture();

    virtual int GetTexID() const override { return 0; }

    virtual int GetWidth() const override { return m_desc.width; }
    virtual int GetHeight() const override { return m_desc.height; }
    virtual int GetDepth() const override { return m_desc.depth; }

    virtual TextureFormat GetFormat() const override { return m_desc.format; }

    virtual void Bind() const override;
    static void UnBind(TextureTarget target);

    virtual void Upload(const void* pixels, int x, int y, int w, int h,
        int miplevel = 0, int row_alignment = 4) override;

    virtual void ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler) override;

private:
	TextureDescription m_desc;

}; // Texture

}
}