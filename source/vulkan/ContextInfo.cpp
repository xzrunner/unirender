#include "unirender/vulkan/ContextInfo.h"
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
#include "unirender/Adaptor.h"

#include <shadertrans/ShaderTrans.h>

#include <iostream>

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

ContextInfo::ContextInfo(VulkanDevice& vk_dev, bool include_depth)
    : m_vk_dev(vk_dev)
    , m_include_depth(include_depth)
    , m_vk_ctx(vk_dev)
{
}

void ContextInfo::Init(int width, int height, void* hwnd)
{
    m_vk_ctx.Init(width, height, hwnd);

    m_vk_dev.m_vk_dev = m_vk_ctx.GetDevice();

    this->width = width;
    this->height = height;

	const bool use_texture = false;
	const bool include_vi = true;

    auto dev = m_vk_ctx.GetDevice();

	swapchain = std::make_shared<Swapchain>(dev);
	swapchain->Create(*this);

    cmd_pool = std::make_shared<CommandPool>(dev);
    cmd_pool->Create();

    cmd_bufs = std::make_shared<CommandBuffers>(dev, cmd_pool);
    cmd_bufs->Create(swapchain->GetImageCount());

	depth_buf = std::make_shared<DepthBuffer>(dev);
	depth_buf->Create(*this);

	uniform_buf = std::make_shared<UniformBuffer>(dev);
	uniform_buf->Create(*this);

    auto single_ubo_binding_point = std::make_shared<DescriptorSetLayout>(dev);
    desc_set_layouts["single_ubo"] = single_ubo_binding_point;
    single_ubo_binding_point->AddBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    single_ubo_binding_point->Create();

    auto single_img_binding_point = std::make_shared<DescriptorSetLayout>(dev);
    desc_set_layouts["single_img"] = single_ubo_binding_point;
    single_img_binding_point->AddBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    single_img_binding_point->Create();

	pipeline_layout = std::make_shared<PipelineLayout>(dev);
    pipeline_layout->AddLayout(single_ubo_binding_point);
    if (use_texture) {
        pipeline_layout->AddLayout(single_img_binding_point);
    }
	pipeline_layout->Create();

	renderpass = std::make_shared<RenderPass>(dev);
	renderpass->Create(*this, m_include_depth);

	frame_buffers = std::make_shared<FrameBuffers>(dev);
	frame_buffers->Create(*this, m_include_depth);

	vert_buf = std::make_shared<vulkan::VertexBuffer>();
    vert_buf->Create(m_vk_ctx, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
        sizeof(g_vb_solid_face_colors_Data[0]), false);
    //uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);
    //vert_buf->Create(vertexBuffer.data(), vertexBufferSize, sizeof(Vertex), false);

    //idx_buf = std::make_shared<vulkan::IndexBuffer>();
    //uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    //idx_buf->Create(vertexBuffer.data(), indexBufferSize);

    std::vector<unsigned int> _vs, _fs;
    shadertrans::ShaderTrans::GLSL2SpirV(Adaptor::ToShaderTransStage(ShaderType::VertexShader), vs, _vs);
    shadertrans::ShaderTrans::GLSL2SpirV(Adaptor::ToShaderTransStage(ShaderType::FragmentShader), fs, _fs);
	program = std::make_shared<vulkan::ShaderProgram>(dev, _vs, _fs);

	desc_pool = std::make_shared<DescriptorPool>(dev);
	desc_pool->Create(use_texture);

	desc_set = std::make_shared<DescriptorSet>(dev);
    desc_set->AddLayout(single_ubo_binding_point);
    desc_set->AddDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &uniform_buf->GetBufferInfo());
    if (use_texture) {
        desc_set->AddLayout(single_img_binding_point);
        desc_set->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &texture_data.image_info);
    }
	desc_set->Create(*desc_pool);

	pipeline_cache = std::make_shared<PipelineCache>(dev);
	pipeline_cache->Create();

	pipeline = std::make_shared<Pipeline>(dev);
	pipeline->Create(*this, m_include_depth, include_vi);
}

void ContextInfo::Resize(uint32_t width, uint32_t height)
{
    m_vk_ctx.Resize(width, height);

    this->width = width;
    this->height = height;

    auto dev = m_vk_ctx.GetDevice();

    vkDeviceWaitIdle(dev);

    swapchain = std::make_shared<Swapchain>(dev);
    swapchain->Create(*this);

    depth_buf = std::make_shared<DepthBuffer>(dev);
    depth_buf->Create(*this);

    frame_buffers = std::make_shared<FrameBuffers>(dev);
    frame_buffers->Create(*this, m_include_depth);

    cmd_bufs = std::make_shared<CommandBuffers>(dev, cmd_pool);
    cmd_bufs->Create(swapchain->GetImageCount());

    vkDeviceWaitIdle(dev);
}

}
}