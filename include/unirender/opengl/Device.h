#pragma once

#include "unirender/Device.h"

#include "unirender/PrimitiveType.h"
#include "unirender/InternalFormat.h"
#include "unirender/AttachmentType.h"

namespace ur
{

class ShaderProgram;
class VertexBuffer;
class IndexBuffer;
class TextureSampler;
class RenderBuffer;
class Framebuffer;

namespace opengl
{

class Device : public ur::Device
{
public:
    Device();

    virtual int GetMaxNumVertAttrs() const override { return m_max_num_vert_attrs; }
    virtual int GetMaxNumTexUnits() const override { return m_max_num_tex_units; }
    virtual int GetMaxNumColorAttachments() const override { return m_max_num_color_attachments; }

    virtual std::shared_ptr<VertexArray>
        GetVertexArray(PrimitiveType prim, VertexLayoutType layout) const override;

    virtual std::shared_ptr<VertexArray> CreateVertexArray() const override;
    virtual std::shared_ptr<Framebuffer> CreateFramebuffer() const override;
    virtual std::shared_ptr<RenderBuffer> CreateRenderBuffer(
        int width, int height, InternalFormat format, AttachmentType attach) const override;

    virtual std::shared_ptr<ur::ShaderProgram>
        CreateShaderProgram(const std::vector<unsigned int>& vs, const std::vector<unsigned int>& fs) const override;
    virtual std::shared_ptr<ur::ShaderProgram>
        CreateShaderProgram(const std::vector<unsigned int>& cs) const override;

    virtual std::shared_ptr<ur::VertexBuffer>
        CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const override;
    virtual std::shared_ptr<ur::IndexBuffer>
        CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const override;
    virtual std::shared_ptr<ur::WritePixelBuffer>
        CreateWritePixelBuffer(BufferUsageHint hint, int size_in_bytes) const override;
    virtual std::shared_ptr<ur::ComputeBuffer>
        CreateComputeBuffer(const std::vector<int>& buf, size_t index) const override;
    virtual std::shared_ptr<ur::ComputeBuffer>
        CreateComputeBuffer(const std::vector<float>& buf, size_t index) const override;

    virtual std::shared_ptr<ur::Texture>
        CreateTexture(const TextureDescription& desc, const void* pixels = nullptr) const override;
    virtual std::shared_ptr<ur::Texture>
        CreateTexture(size_t width, size_t height, ur::TextureFormat format, const void* buf, size_t buf_sz) const override;
    virtual std::shared_ptr<ur::Texture>
        CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const override;
    virtual std::shared_ptr<TextureSampler>
        CreateTextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter, TextureWrap wrap_s, TextureWrap wrap_t) const override;
    virtual std::shared_ptr<TextureSampler>
        GetTextureSampler(TextureSamplerType type) const override;

    virtual void DispatchCompute(int thread_group_count) const override;

    virtual void ReadPixels(const unsigned char* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const override;
    virtual void ReadPixels(const short* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const override;

private:
    void Init();

    std::shared_ptr<VertexArray>
        CreateQuadVertexArray(VertexLayoutType type) const;
    std::shared_ptr<VertexArray>
        CreateCubeVertexArray(VertexLayoutType type) const;

private:
    int m_max_num_vert_attrs        = 0;
    int m_max_num_tex_units         = 0;
    int m_max_num_color_attachments = 0;

    std::shared_ptr<TextureSampler> m_nearest_clamp  = nullptr;
    std::shared_ptr<TextureSampler> m_linear_clamp   = nullptr;
    std::shared_ptr<TextureSampler> m_nearest_repeat = nullptr;
    std::shared_ptr<TextureSampler> m_linear_repeat  = nullptr;

    mutable std::shared_ptr<VertexArray> m_cube_va[static_cast<int>(VertexLayoutType::MaxCount)];
    mutable std::shared_ptr<VertexArray> m_quad_va[static_cast<int>(VertexLayoutType::MaxCount)];

}; // Device

}
}