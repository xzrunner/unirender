#include "unirender/vulkan/ContextInfo.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/CommandBuffers.h"
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/DeviceInfo.h"
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
#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

layout (location = 0) out vec3 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() 
{
	outColor = inColor;
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
)";

const char* fs = R"(
#version 450

layout (location = 0) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

void main() 
{
  outFragColor = vec4(inColor, 1.0);
}
)";

}

namespace ur
{
namespace vulkan
{

void ContextInfo::Init(const ur::Device& device)
{
	const bool include_depth = false;
	const bool use_texture = false;
	const bool include_vi = true;

	auto& vk_dev = static_cast<const vulkan::Device&>(device);
	auto& dev_info = vk_dev.GetInfo();
	auto dev = dev_info.device;

	cmd_pool = std::make_shared<CommandPool>(dev);
	cmd_pool->Create();

	swapchain = std::make_shared<Swapchain>(dev);
	swapchain->Create(dev_info);

    cmd_bufs = std::make_shared<CommandBuffers>(dev, cmd_pool);
    cmd_bufs->Create(swapchain->GetImageCount());

	depth_buf = std::make_shared<DepthBuffer>(dev);
	depth_buf->Create(dev_info);

	uniform_buf = std::make_shared<UniformBuffer>(dev);
	uniform_buf->Create(dev_info);

	desc_set_layout = std::make_shared<DescriptorSetLayout>(dev);
	desc_set_layout->Create(use_texture);

	pipeline_layout = std::make_shared<PipelineLayout>(dev);
	pipeline_layout->Create(*desc_set_layout);

	renderpass = std::make_shared<RenderPass>(dev);
	renderpass->Create(dev_info, *this, include_depth);

	frame_buffers = std::make_shared<FrameBuffers>(dev);
	frame_buffers->Create(dev_info, *this, include_depth);

	vert_buf = std::make_shared<vulkan::VertexBuffer>();
    vert_buf->Create(dev_info, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
        sizeof(g_vb_solid_face_colors_Data[0]), false);
	program = std::make_shared<vulkan::ShaderProgram>(dev, vs, fs);

	desc_pool = std::make_shared<DescriptorPool>(dev);
	desc_pool->Create(use_texture);

	desc_set = std::make_shared<DescriptorSet>(dev);
	desc_set->Create(dev_info, *this, use_texture);

	pipeline_cache = std::make_shared<PipelineCache>(dev);
	pipeline_cache->Create();

	pipeline = std::make_shared<Pipeline>(dev);
	pipeline->Create(*this, include_depth, include_vi);
}

}
}