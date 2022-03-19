#pragma once

#include "unirender/Texture.h"
#include "unirender/TextureDescription.h"
#include "unirender/TextureTarget.h"
#include "unirender/opengl/opengl.h"

#include <memory>

namespace ur
{

class Device;
class TextureSampler;
class WritePixelBuffer;
class ReadPixelBuffer;

namespace opengl
{

class Texture : public ur::Texture
{
public:
    Texture(TextureDescription desc, const ur::Device& device);
    virtual ~Texture();

    virtual int GetTexID() const override { return m_id; }

    virtual int GetWidth() const override { return m_desc.width; }
    virtual int GetHeight() const override { return m_desc.height; }
    virtual int GetDepth() const override { return m_desc.depth; }

    virtual TextureTarget GetTarget() const override { return m_desc.target; }
    virtual ur::TextureFormat GetFormat() const override { return m_desc.format; }

    virtual void Bind() const override;
    static void UnBind(TextureTarget target);

    virtual void Upload(const void* pixels, int x, int y, int w, int h,
        int miplevel = 0, int row_alignment = 4) override;

    virtual void ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler) override;

    virtual void BindToImage(uint32_t unit, AccessType access) const override;

    virtual void* WriteToMemory(int size) const override;
    virtual void WriteToMemory(void* data) const override;

    bool ReadFromMemory(const WritePixelBuffer& buf, int x, int y,
        int w, int h, int row_alignment);
    void ReadFromMemory(const void* pixels, ur::TextureFormat fmt,
        int width, int height, int depth, int row_alignment, int mip_level = 0);

    std::shared_ptr<ur::ReadPixelBuffer> Texture::WriteToMemory(int row_alignment);

private:
    void BindToLastTextureUnit() const;

    static void VerifyRowAlignment(int row_alignment);

private:
    GLuint m_id;

    TextureDescription m_desc;

    GLenum m_last_tex_unit;

}; // Texture

}
}