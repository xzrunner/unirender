#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/VertexArray.h"
#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/Texture.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/ValidationLayers.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/Instance.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/Pipeline.h"
#include "unirender/vulkan/TextureSampler.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/Framebuffer.h"
#include "unirender/vulkan/RenderBuffer.h"
#include "unirender/vulkan/TypeConverter.h"
#include "unirender/TextureDescription.h"
#include "unirender/ComponentDataType.h"
#include "unirender/VertexInputAttribute.h"

#include <iostream>
#include <cassert>

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

    // Pick a physical device (no surface needed -- headless selection)
    m_phy_dev = std::make_shared<PhysicalDevice>(*m_instance);

    // FIX: Create logical device eagerly so that buffer/texture creation
    //      works even before a Context (window surface) exists.
    //      Present queue will be null, which is fine for off-screen use.
    m_logic_dev = std::make_shared<LogicalDevice>(
        enable_validation_layers, *m_phy_dev, /*surface=*/nullptr);

    // Create a command pool for resource-upload operations. Its queue family must be
    // the graphics family the upload command buffers are submitted to (resolved here
    // the same way LogicalDevice picked the graphics queue), not a hardcoded 0.
    {
        PhysicalDevice::QueueFamilyIndices qfi =
            PhysicalDevice::FindQueueFamilies(m_phy_dev->GetHandler(), nullptr);
        m_cmd_pool = std::make_shared<CommandPool>(m_logic_dev,
            qfi.graphics_family.value());
    }

    // Query limits from physical device
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_phy_dev->GetHandler(), &props);
    m_max_num_vert_attrs        = static_cast<int>(props.limits.maxVertexInputAttributes);
    m_max_num_tex_units         = static_cast<int>(props.limits.maxPerStageDescriptorSampledImages);
    m_max_num_color_attachments = static_cast<int>(props.limits.maxColorAttachments);
    m_max_num_img_units         = static_cast<int>(props.limits.maxPerStageDescriptorStorageImages);
}

// ---------------------------------------------------------------------------
// VertexArray
// ---------------------------------------------------------------------------

std::shared_ptr<ur::VertexArray>
Device::GetVertexArray(PrimitiveType prim, VertexLayoutType layout, bool unit) const
{
    const int layout_idx = static_cast<int>(layout);
    if (layout_idx < 0 || layout_idx >= (int)VertexLayoutType::MaxCount) {
        return nullptr;
    }
    switch (prim)
    {
    case PrimitiveType::Quad:
        if (!m_quad_va[layout_idx]) {
            m_quad_va[layout_idx] = CreateQuadVertexArray(layout, unit);
        }
        return m_quad_va[layout_idx];
    case PrimitiveType::Cube:
        if (!m_cube_va[layout_idx]) {
            m_cube_va[layout_idx] = CreateCubeVertexArray(layout, unit);
        }
        return m_cube_va[layout_idx];
    default:
        return nullptr;
    }
}

// Full-screen quad: 4-vertex tri_strip, NO index buffer (drawn with tri_strip).
// Geometry matches the GL / Metal backends' CreateQuadVertexArray so the post-process
// passes (outline edge-detect, FXAA, composite) sample the same UVs. Without this the
// Vulkan GetVertexArray stub returned null and every post-process Draw was a no-op,
// leaving the composited 3D scene blank on screen (only the 2D GUI showed).
std::shared_ptr<ur::VertexArray>
Device::CreateQuadVertexArray(VertexLayoutType layout, bool unit) const
{
    const float p_min = unit ? 0.0f : -1.0f;

    std::vector<float> vertices;
    switch (layout)
    {
    case VertexLayoutType::Pos:
        vertices = {
            p_min, 1.0f,  0.0f,
            p_min, p_min, 0.0f,
            1.0f,  1.0f,  0.0f,
            1.0f,  p_min, 0.0f,
        };
        break;
    case VertexLayoutType::PosTex:
        vertices = {
            p_min, 1.0f,  0.0f, 0.0f, 1.0f,
            p_min, p_min, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
            1.0f,  p_min, 0.0f, 1.0f, 0.0f,
        };
        break;
    case VertexLayoutType::PosNorm:
        vertices = {
            p_min, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f,
            p_min, p_min, 0.0f, 0.0f, 0.0f, 1.0f,
            1.0f,  1.0f,  0.0f, 0.0f, 0.0f, 1.0f,
            1.0f,  p_min, 0.0f, 0.0f, 0.0f, 1.0f,
        };
        break;
    case VertexLayoutType::PosNormTex:
        vertices = {
            p_min, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            p_min, p_min, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            1.0f,  p_min, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        };
        break;
    default:
        // PosNormTexTB and any others are not used by the post-process quads.
        return nullptr;
    }

    auto va = CreateVertexArray();

    int vbuf_sz = (int)(sizeof(float) * vertices.size());
    auto vbuf = CreateVertexBuffer(BufferUsageHint::StaticDraw, vbuf_sz);
    vbuf->ReadFromMemory(vertices.data(), vbuf_sz, 0);
    va->SetVertexBuffer(vbuf);

    std::vector<std::shared_ptr<ur::VertexInputAttribute>> attrs;
    switch (layout)
    {
    case VertexLayoutType::Pos:
        attrs.push_back(std::make_shared<ur::VertexInputAttribute>(0, ComponentDataType::Float, 3, 0, 12));
        break;
    case VertexLayoutType::PosTex:
        attrs.push_back(std::make_shared<ur::VertexInputAttribute>(0, ComponentDataType::Float, 3, 0, 20));
        attrs.push_back(std::make_shared<ur::VertexInputAttribute>(1, ComponentDataType::Float, 2, 12, 20));
        break;
    case VertexLayoutType::PosNorm:
        attrs.push_back(std::make_shared<ur::VertexInputAttribute>(0, ComponentDataType::Float, 3, 0, 24));
        attrs.push_back(std::make_shared<ur::VertexInputAttribute>(1, ComponentDataType::Float, 3, 12, 24));
        break;
    case VertexLayoutType::PosNormTex:
        attrs.push_back(std::make_shared<ur::VertexInputAttribute>(0, ComponentDataType::Float, 3, 0, 32));
        attrs.push_back(std::make_shared<ur::VertexInputAttribute>(1, ComponentDataType::Float, 3, 12, 32));
        attrs.push_back(std::make_shared<ur::VertexInputAttribute>(2, ComponentDataType::Float, 2, 24, 32));
        break;
    default:
        break;
    }
    va->SetVertexBufferAttrs(attrs);

    return va;
}

std::shared_ptr<ur::VertexArray>
Device::CreateCubeVertexArray(VertexLayoutType layout, bool unit) const
{
    // Not used by the deferred / post-process path; build on demand if a scene needs it.
    return nullptr;
}

std::shared_ptr<ur::VertexArray>
Device::CreateVertexArray() const
{
    return std::make_shared<ur::vulkan::VertexArray>(*this);
}

// ---------------------------------------------------------------------------
// Framebuffer / RenderBuffer
// ---------------------------------------------------------------------------

std::shared_ptr<ur::Framebuffer>
Device::CreateFramebuffer() const
{
    return std::make_shared<ur::vulkan::Framebuffer>(m_logic_dev, m_phy_dev);
}

std::shared_ptr<ur::RenderBuffer>
Device::CreateRenderBuffer(int width, int height, InternalFormat format) const
{
    return std::make_shared<ur::vulkan::RenderBuffer>(width, height, format);
}

// ---------------------------------------------------------------------------
// ShaderProgram -- expects SPIR-V input
// ---------------------------------------------------------------------------

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::vector<unsigned int>& vs,
                            const std::vector<unsigned int>& fs,
                            const std::vector<unsigned int>& tcs,
                            const std::vector<unsigned int>& tes,
                            const std::vector<unsigned int>& gs) const
{
    if (vs.empty() || fs.empty()) {
        return nullptr;
    }
    return std::make_shared<ur::vulkan::ShaderProgram>(m_logic_dev, *m_phy_dev, vs, fs);
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::vector<unsigned int>& cs) const
{
    if (cs.empty()) return nullptr;
    // TODO: compute shader
    return nullptr;
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::string& cs) const
{
    return nullptr; // GLSL string not directly supported in Vulkan
}

// ---------------------------------------------------------------------------
// Buffers
// ---------------------------------------------------------------------------

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
    return std::make_shared<ur::vulkan::VertexBuffer>(m_logic_dev, m_phy_dev, m_cmd_pool);
}

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(const void* data, size_t size) const
{
    auto vb = std::make_shared<ur::vulkan::VertexBuffer>(m_logic_dev, m_phy_dev, m_cmd_pool);
    if (data && size > 0) {
        vb->ReadFromMemory(data, static_cast<int>(size), 0);
    }
    return vb;
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
Device::CreateComputeBuffer(const void* data, size_t size, size_t index) const
{
    return nullptr;
}

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------

std::shared_ptr<ur::Texture>
Device::CreateTexture(const TextureDescription& desc, const void* pixels) const
{
    auto sampler = GetTextureSampler(desc.sampler_type);
    auto tex = std::make_shared<ur::vulkan::Texture>(m_logic_dev, m_phy_dev, sampler);
    if (pixels) {
        tex->ReadFromMemory(desc, m_cmd_pool, pixels, 4);
    } else {
        // No source pixels -> likely a render target (dtex atlas, rendergraph
        // render_target). Make it a color attachment ONLY if the device can
        // actually render to that format -- compressed (ETC2/PVRTC/S3TC) and other
        // non-renderable formats must NOT get COLOR_ATTACHMENT usage (MoltenVK
        // errors, e.g. "ETC2... cannot be cleared"). Those stay imageless, as
        // before, until uploaded.
        VkFormatProperties props = {};
        vkGetPhysicalDeviceFormatProperties(
            m_phy_dev->GetHandler(), TypeConverter::To(desc.format), &props);
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            tex->CreateAsColorAttachment(desc, m_cmd_pool);
        }
    }
    return tex;
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(size_t width, size_t height, TextureFormat format,
                      const void* buf, size_t buf_sz, bool gamma_correction) const
{
    TextureDescription desc;
    desc.target = ur::TextureTarget::Texture2D;
    desc.width  = static_cast<int>(width);
    desc.height = static_cast<int>(height);
    desc.format = format;
    desc.gamma_correction = gamma_correction;
    return CreateTexture(desc, buf);
}

std::shared_ptr<ur::Texture>
Device::CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const
{
    return nullptr; // TODO
}

std::shared_ptr<ur::TextureSampler>
Device::CreateTextureSampler(TextureMinificationFilter min_filter,
                             TextureMagnificationFilter mag_filter,
                             TextureWrap wrap_s, TextureWrap wrap_t,
                             float max_anistropy) const
{
    return std::make_shared<vulkan::TextureSampler>(
        m_logic_dev, wrap_s, wrap_t, wrap_s, min_filter, mag_filter, max_anistropy);
}

// ---------------------------------------------------------------------------
// Uniform / Descriptor
// ---------------------------------------------------------------------------

std::shared_ptr<ur::UniformBuffer>
Device::CreateUniformBuffer(const void* data, size_t size) const
{
    return std::make_shared<UniformBuffer>(m_logic_dev, *m_phy_dev, data, size);
}

std::shared_ptr<ur::DescriptorPool>
Device::CreateDescriptorPool(size_t max_sets,
    const std::vector<std::pair<DescriptorType, size_t>>& pool_sizes) const
{
    return std::make_shared<DescriptorPool>(m_logic_dev, max_sets, pool_sizes);
}

std::shared_ptr<ur::DescriptorSetLayout>
Device::CreateDescriptorSetLayout(
    const std::vector<std::pair<DescriptorType, ShaderType>>& bindings) const
{
    return std::make_shared<DescriptorSetLayout>(m_logic_dev, bindings);
}

std::shared_ptr<ur::DescriptorSet>
Device::CreateDescriptorSet(const ur::DescriptorPool& pool,
    const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& layouts,
    const std::vector<ur::Descriptor>& descriptors) const
{
    return std::make_shared<DescriptorSet>(*this, m_logic_dev, pool, layouts, descriptors);
}

// ---------------------------------------------------------------------------
// Compute
// ---------------------------------------------------------------------------

void Device::DispatchCompute(int num_groups_x, int num_groups_y, int num_groups_z) const
{
    // TODO
}

// ---------------------------------------------------------------------------
// ReadPixels
// ---------------------------------------------------------------------------

void Device::ReadPixels(const unsigned char* pixels, ur::TextureFormat fmt,
                        int x, int y, int w, int h) const
{
    // TODO: blit framebuffer attachment into a host-visible staging buffer
}

void Device::ReadPixels(const short* pixels, ur::TextureFormat fmt,
                        int x, int y, int w, int h) const
{
    // TODO
}

// ---------------------------------------------------------------------------
// Debug markers (require VK_EXT_debug_utils)
// ---------------------------------------------------------------------------

void Device::PushDebugGroup(const std::string& msg) const {}
void Device::PopDebugGroup() const {}

}
}
