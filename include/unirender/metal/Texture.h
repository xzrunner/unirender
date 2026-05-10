#pragma once

#include "unirender/Texture.h"
#include "unirender/TextureDescription.h"

#include <memory>

namespace ur
{
namespace metal
{

class Texture : public ur::Texture
{
public:
    Texture(void* mtl_device, const TextureDescription& desc);
    virtual ~Texture();

    virtual int GetTexID() const override { return 0; }

    virtual int GetWidth()  const override { return m_desc.width; }
    virtual int GetHeight() const override { return m_desc.height; }
    virtual int GetDepth()  const override { return m_desc.depth; }

    virtual TextureTarget GetTarget() const override { return m_desc.target; }
    virtual TextureFormat GetFormat() const override { return m_desc.format; }

    virtual void Bind() const override {}

    virtual void Upload(const void* pixels, int x, int y, int w, int h,
        int miplevel = 0, int row_alignment = 4) override;

    virtual void ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler) override;

    virtual void BindToImage(uint32_t unit, AccessType access) const override {}

    virtual void* WriteToMemory(int size) const override;
    virtual void WriteToMemory(void* data) const override;

    void ReadFromMemory(const void* pixels, TextureFormat fmt,
                        int width, int height, int depth, int row_alignment);

    void* GetMTLTexture() const { return m_mtl_texture; }

private:
    void CreateMTLTexture();

    void* m_mtl_device  = nullptr;  // id<MTLDevice>
    void* m_mtl_texture = nullptr;  // id<MTLTexture>

    TextureDescription m_desc;

}; // Texture

}
}
