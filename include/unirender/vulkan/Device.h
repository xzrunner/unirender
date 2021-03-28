#pragma once

#include "unirender/Device.h"

namespace ur
{
namespace vulkan
{

class Instance;
class ValidationLayers;
class PhysicalDevice;
class LogicalDevice;
class CommandPool;

class Device : public ur::Device
{
public:
    Device(bool enable_validation_layers);

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
        CreateTexture(size_t width, size_t height, TextureFormat format, const void* buf, size_t buf_sz, bool gamma_correction = false) const override;
	virtual std::shared_ptr<ur::Texture>
		CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const override;
    virtual std::shared_ptr<TextureSampler> CreateTextureSampler(TextureMinificationFilter min_filter,
        TextureMagnificationFilter mag_filter, TextureWrap wrap_s, TextureWrap wrap_t, float max_anistropy = 1.0) const override;

    virtual std::shared_ptr<ur::UniformBuffer>
        CreateUniformBuffer(const void* data, size_t size) const override;
    virtual std::shared_ptr<ur::DescriptorPool>
        CreateDescriptorPool(size_t max_sets, const std::vector<std::pair<DescriptorType, size_t>>& pool_sizes) const override;
    virtual std::shared_ptr<ur::DescriptorSetLayout>
        CreateDescriptorSetLayout(const std::vector<std::pair<DescriptorType, ShaderType>>& bindings) const override;
    virtual std::shared_ptr<ur::DescriptorSet> CreateDescriptorSet(const ur::DescriptorPool& pool,
        const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& layouts,
        const std::vector<ur::Descriptor>& descriptors) const override;
    virtual std::shared_ptr<ur::VertexBuffer>
        CreateVertexBuffer(const void* data, size_t size) const override;

    virtual void DispatchCompute(int num_groups_x, int num_groups_y, int num_groups_z) const override;

    virtual void ReadPixels(const unsigned char* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const override;
    virtual void ReadPixels(const short* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const override;

private:
    int m_max_num_vert_attrs = 0;
    int m_max_num_tex_units = 0;
    int m_max_num_color_attachments = 0;
    int m_max_num_img_units = 0;

    bool m_enable_validation_layers = false;

    std::shared_ptr<Instance> m_instance = nullptr;
    std::shared_ptr<ValidationLayers> m_valid_layers = nullptr;

    std::shared_ptr<PhysicalDevice> m_phy_dev = nullptr;
    std::shared_ptr<LogicalDevice>  m_logic_dev = nullptr;
    uint32_t m_present_family_id = 0;

    std::shared_ptr<CommandPool> m_cmd_pool = nullptr;

    friend class Context;

}; // Device
}
}