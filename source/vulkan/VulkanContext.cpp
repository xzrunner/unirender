#include "unirender/vulkan/VulkanContext.h"
#include "unirender/vulkan/VulkanDevice.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/CommandBuffers.h"
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/FrameBuffers.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/PipelineCache.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/PipelineLayout.h"
#include "unirender/vulkan/Pipeline.h"
#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/Surface.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/Adaptor.h"

#include <set>

namespace
{

struct Vertex {
    float posX, posY, posZ, posW;  // Position data
    float r, g, b, a;              // Color
};

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f
#define UV(_u_, _v_) (_u_), (_v_)

static const Vertex g_vb_solid_face_colors_Data[] = {
    // red face
    {XYZ1(-1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    // green face
    {XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    // blue face
    {XYZ1(-1, 1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 1.f)},
    // yellow face
    {XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(1.f, 1.f, 0.f)},
    // magenta face
    {XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    // cyan face
    {XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
};

const char* vs = R"(
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (std140, binding = 0) uniform bufferVals {
    mat4 mvp;
} myBufferVals;
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;
layout (location = 0) out vec4 outColor;
void main() {
   outColor = inColor;
   gl_Position = myBufferVals.mvp * pos;
}
)";

const char* fs = R"(
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec4 color;
layout (location = 0) out vec4 outColor;
void main() {
    outColor = color;
}
)";

}

namespace ur
{
namespace vulkan
{

VulkanContext::VulkanContext(const VulkanDevice& vk_dev)
    : m_vk_dev(vk_dev)
{
}

void VulkanContext::Init(uint32_t width, uint32_t height, void* hwnd)
{
    m_surface = std::make_shared<Surface>(m_vk_dev.GetInstance());
    m_surface->Create(hwnd);

    m_phy_dev = std::make_shared<PhysicalDevice>(m_vk_dev.GetInstance(), m_surface->GetHandler());
    m_phy_dev->Create();

    m_logic_dev = std::make_shared<LogicalDevice>();
    m_logic_dev->Create(m_vk_dev, *m_surface, *m_phy_dev);

    auto logic_dev = m_logic_dev->GetHandler();
    m_vk_dev.m_vk_dev = logic_dev;

    m_width = width;
    m_height = height;

	const bool use_texture = false;
	const bool include_vi = true;

    m_swapchain = std::make_shared<Swapchain>(logic_dev);
    m_swapchain->Create(*this);

    m_cmd_pool = std::make_shared<CommandPool>(logic_dev);
    m_cmd_pool->Create();

    m_cmd_bufs = std::make_shared<CommandBuffers>(logic_dev, m_cmd_pool);
    m_cmd_bufs->Create(m_swapchain->GetImageCount());

    m_depth_buf = std::make_shared<DepthBuffer>(logic_dev);
    m_depth_buf->Create(*this);

    m_uniform_buf = std::make_shared<UniformBuffer>(logic_dev);
    m_uniform_buf->Create(*this);

    auto single_ubo_binding_point = std::make_shared<DescriptorSetLayout>(logic_dev);
    m_desc_set_layouts["single_ubo"] = single_ubo_binding_point;
    single_ubo_binding_point->AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    single_ubo_binding_point->Create();

    auto single_img_binding_point = std::make_shared<DescriptorSetLayout>(logic_dev);
    m_desc_set_layouts["single_img"] = single_ubo_binding_point;
    single_img_binding_point->AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    single_img_binding_point->Create();

    m_pipeline_layout = std::make_shared<PipelineLayout>(logic_dev);
    m_pipeline_layout->AddLayout(single_ubo_binding_point);
    if (use_texture) {
        m_pipeline_layout->AddLayout(single_img_binding_point);
    }
    m_pipeline_layout->Create();

    m_renderpass = std::make_shared<RenderPass>(logic_dev);
    m_renderpass->Create(*this, m_include_depth);

    m_frame_buffers = std::make_shared<FrameBuffers>(logic_dev);
    m_frame_buffers->Create(*this, m_include_depth);

    m_vert_buf = std::make_shared<vulkan::VertexBuffer>();
    m_vert_buf->Create(*this, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
        sizeof(g_vb_solid_face_colors_Data[0]), false);
    //uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);
    //vert_buf->Create(vertexBuffer.data(), vertexBufferSize, sizeof(Vertex), false);

    //idx_buf = std::make_shared<vulkan::IndexBuffer>();
    //uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    //idx_buf->Create(vertexBuffer.data(), indexBufferSize);

    std::vector<unsigned int> _vs, _fs;
    shadertrans::ShaderTrans::GLSL2SpirV(Adaptor::ToShaderTransStage(ShaderType::VertexShader), vs, _vs);
    shadertrans::ShaderTrans::GLSL2SpirV(Adaptor::ToShaderTransStage(ShaderType::FragmentShader), fs, _fs);
    m_program = std::make_shared<vulkan::ShaderProgram>(logic_dev, _vs, _fs);

    m_desc_pool = std::make_shared<DescriptorPool>(logic_dev);
    m_desc_pool->Create(use_texture);

    m_desc_set = std::make_shared<DescriptorSet>(logic_dev);
    m_desc_set->AddLayout(single_ubo_binding_point);
    m_desc_set->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_uniform_buf->GetBufferInfo());
    if (use_texture) {
        m_desc_set->AddLayout(single_img_binding_point);
        m_desc_set->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &m_texture_data.image_info);
    }
    m_desc_set->Create(*m_desc_pool);

    m_pipeline_cache = std::make_shared<PipelineCache>(logic_dev);
    m_pipeline_cache->Create();

    m_pipeline = std::make_shared<Pipeline>(logic_dev);
    m_pipeline->Create(*this, m_include_depth, include_vi);
}

void VulkanContext::Resize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;

    auto logic_dev = m_logic_dev->GetHandler();

    vkDeviceWaitIdle(logic_dev);

    m_swapchain = std::make_shared<Swapchain>(logic_dev);
    m_swapchain->Create(*this);

    m_depth_buf = std::make_shared<DepthBuffer>(logic_dev);
    m_depth_buf->Create(*this);

    m_frame_buffers = std::make_shared<FrameBuffers>(logic_dev);
    m_frame_buffers->Create(*this, m_include_depth);

    m_cmd_bufs = std::make_shared<CommandBuffers>(logic_dev, m_cmd_pool);
    m_cmd_bufs->Create(m_swapchain->GetImageCount());

    vkDeviceWaitIdle(logic_dev);
}

}
}