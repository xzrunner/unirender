#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/VertexArray.h"
#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/Texture.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/ValidationLayers.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Instance.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/Pipeline.h"
#include "unirender/vulkan/TextureSampler.h"

#include <iostream>

#include <assert.h>

namespace ur
{
namespace vulkan
{

Device::Device(bool enable_validation_layers)
    : m_enable_validation_layers(enable_validation_layers)
{
    m_instance = std::make_shared<Instance>(enable_validation_layers);

    if (enable_validation_layers) {
        m_valid_layers = std::make_shared<ValidationLayers>(m_instance);
    }

    m_phy_dev = std::make_shared<PhysicalDevice>(*m_instance);
    //m_logic_dev = std::make_shared<LogicalDevice>(enable_validation_layers, *m_phy_dev);
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
Device::CreateRenderBuffer(int width, int height, InternalFormat format) const
{
    return nullptr;
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::vector<unsigned int>& vs, 
                            const std::vector<unsigned int>& fs,
                            const std::vector<unsigned int>& tcs,
                            const std::vector<unsigned int>& tes,
                            const std::vector<unsigned int>& gs) const
{
    if (vs.empty() || fs.empty()) {
        return nullptr;
    } else {
        return std::make_shared<ur::vulkan::ShaderProgram>(m_logic_dev, vs, fs);
    }
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::vector<unsigned int>& cs) const
{
    return nullptr;
}

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
	return std::make_shared<ur::vulkan::VertexBuffer>(m_logic_dev, m_phy_dev, m_cmd_pool);
}

std::shared_ptr<ur::IndexBuffer>
Device::CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
	return std::make_shared<ur::vulkan::IndexBuffer>(m_logic_dev, m_phy_dev, m_cmd_pool);
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
    auto smaple = GetTextureSampler(desc.sampler_type);
	auto tex = std::make_shared<ur::vulkan::Texture>(m_logic_dev, m_phy_dev, smaple);
    if (pixels) {
        tex->ReadFromMemory(desc, m_cmd_pool, pixels, 4);
    }
    return tex;
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(size_t width, size_t height, TextureFormat format, const void* buf, size_t buf_sz, bool gamma_correction) const
{
    TextureDescription desc;
    desc.target = ur::TextureTarget::Texture2D;
    desc.width  = width;
    desc.height = height;
    desc.format = format;
    desc.gamma_correction = gamma_correction;
    return CreateTexture(desc, buf);
}

std::shared_ptr<ur::TextureSampler>
Device::CreateTextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter, 
                             TextureWrap wrap_s, TextureWrap wrap_t, float max_anistropy) const
{
    return std::make_shared<vulkan::TextureSampler>(m_logic_dev, wrap_s, wrap_t, wrap_s, min_filter, mag_filter, max_anistropy);
}

std::shared_ptr<ur::Texture>
Device::CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const
{
	return nullptr;
}

std::shared_ptr<ur::UniformBuffer>
Device::CreateUniformBuffer(const void* data, size_t size) const
{
    return std::make_shared<UniformBuffer>(m_logic_dev, *m_phy_dev, data, size);
}

std::shared_ptr<ur::DescriptorPool>
Device::CreateDescriptorPool(size_t max_sets, const std::vector<std::pair<DescriptorType, size_t>>& pool_sizes) const
{
    return std::make_shared<DescriptorPool>(m_logic_dev, max_sets, pool_sizes);
}

std::shared_ptr<ur::DescriptorSetLayout>
Device::CreateDescriptorSetLayout(const std::vector<std::pair<ur::DescriptorType, ur::ShaderType>>& bindings) const
{
    return std::make_shared<DescriptorSetLayout>(m_logic_dev, bindings);
}

std::shared_ptr<ur::DescriptorSet> 
Device::CreateDescriptorSet(const ur::DescriptorPool& pool, const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& layouts,
                            const std::vector<ur::Descriptor>& descriptors) const
{
    return std::make_shared<DescriptorSet>(*this, m_logic_dev, pool, layouts, descriptors);
}

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(const void* data, size_t size) const
{
    auto vb = std::make_shared<VertexBuffer>(m_logic_dev, m_phy_dev, m_cmd_pool);
    vb->ReadFromMemory(data, size, 0);
    return vb;
}

void Device::DispatchCompute(int num_groups_x, int num_groups_y, int num_groups_z) const
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