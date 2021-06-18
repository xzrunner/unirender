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
    Device(std::ostream& logger = std::cerr);

    virtual int GetMaxNumVertAttrs() const override { return m_max_num_vert_attrs; }
    virtual int GetMaxNumTexUnits() const override { return m_max_num_tex_units; }
    virtual int GetMaxNumColorAttachments() const override { return m_max_num_color_attachments; }
    virtual int GetMaxNumImgUnits() const override { return m_max_num_img_units; }

    virtual std::shared_ptr<VertexArray>
        GetVertexArray(PrimitiveType prim, VertexLayoutType layout, bool unit = false) const override;

    virtual std::shared_ptr<VertexArray> CreateVertexArray() const override;
    virtual std::shared_ptr<Framebuffer> CreateFramebuffer() const override;
    virtual std::shared_ptr<RenderBuffer> CreateRenderBuffer(
        int width, int height, InternalFormat format) const override;

    virtual std::shared_ptr<ur::ShaderProgram> CreateShaderProgram(
        const std::vector<unsigned int>& vs, 
        const std::vector<unsigned int>& fs,
        const std::vector<unsigned int>& tcs = std::vector<unsigned int>(),
        const std::vector<unsigned int>& tes = std::vector<unsigned int>(),
        const std::vector<unsigned int>& gs = std::vector<unsigned int>()
    ) const override;
    virtual std::shared_ptr<ur::ShaderProgram>
        CreateShaderProgram(const std::vector<unsigned int>& cs) const override;
    virtual std::shared_ptr<ur::ShaderProgram>
        CreateShaderProgram(const std::string& cs) const override;

    virtual std::shared_ptr<ur::VertexBuffer>
        CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const override;
    virtual std::shared_ptr<ur::IndexBuffer>
        CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const override;
    virtual std::shared_ptr<ur::WritePixelBuffer>
        CreateWritePixelBuffer(BufferUsageHint hint, int size_in_bytes) const override;
    virtual std::shared_ptr<ur::ComputeBuffer>
        CreateComputeBuffer(const void* data, size_t size, size_t index) const override;

    virtual std::shared_ptr<ur::Texture>
        CreateTexture(const TextureDescription& desc, const void* pixels = nullptr) const override;
    virtual std::shared_ptr<ur::Texture>
        CreateTexture(size_t width, size_t height, ur::TextureFormat format, const void* buf, size_t buf_sz, bool gamma_correction = false) const override;
    virtual std::shared_ptr<ur::Texture>
        CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const override;
    virtual std::shared_ptr<TextureSampler> CreateTextureSampler(TextureMinificationFilter min_filter, 
        TextureMagnificationFilter mag_filter, TextureWrap wrap_s, TextureWrap wrap_t, float max_anistropy = 1.0) const override;
    virtual std::shared_ptr<VertexBuffer>
        CreateVertexBuffer(const void* data, size_t size) const override { return nullptr; }

    virtual std::shared_ptr<UniformBuffer>
        CreateUniformBuffer(const void* data, size_t size) const { return nullptr; }
    virtual std::shared_ptr<ur::DescriptorPool>
        CreateDescriptorPool(size_t max_sets, const std::vector<std::pair<DescriptorType, size_t>>& pool_sizes) const override { return nullptr; }
    virtual std::shared_ptr<ur::DescriptorSetLayout>
        CreateDescriptorSetLayout(const std::vector<std::pair<DescriptorType, ShaderType>>& bindings) const override { return nullptr; }
    virtual std::shared_ptr<ur::DescriptorSet> CreateDescriptorSet(const ur::DescriptorPool& pool,
        const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& layouts,
        const std::vector<ur::Descriptor>& descriptors) const override { return nullptr; }

    virtual void DispatchCompute(int num_groups_x, int num_groups_y, int num_groups_z) const override;

    virtual void ReadPixels(const unsigned char* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const override;
    virtual void ReadPixels(const short* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const override;

private:
    void Init();

    std::shared_ptr<VertexArray>
        CreateQuadVertexArray(VertexLayoutType type, bool unit = false) const;
    std::shared_ptr<VertexArray>
        CreateCubeVertexArray(VertexLayoutType type, bool unit = false) const;

private:
    int m_max_num_vert_attrs        = 0;
    int m_max_num_tex_units         = 0;
    int m_max_num_color_attachments = 0;
    int m_max_num_img_units         = 0;

    mutable std::shared_ptr<VertexArray> m_cube_va[static_cast<int>(VertexLayoutType::MaxCount)];
    mutable std::shared_ptr<VertexArray> m_quad_va[static_cast<int>(VertexLayoutType::MaxCount)];

}; // Device

}
}