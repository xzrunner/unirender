#include "unirender/vulkan/Context.h"
#include "unirender/vulkan/Utility.h"
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
#include "unirender/vulkan/Instance.h"
#include "unirender/Adaptor.h"

#include <vulkan/vulkan.h>

#include <iostream>

#include <assert.h>

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

Context::Context(const ur::Device& device, void* hwnd,
	             uint32_t width, uint32_t height)
	: m_dev(static_cast<const vulkan::Device&>(device))
{
	Init(width, height, hwnd);

    //VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
    //imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    //imageAcquiredSemaphoreCreateInfo.pNext = NULL;
    //imageAcquiredSemaphoreCreateInfo.flags = 0;

    //VkResult res = vkCreateSemaphore(vk_dev, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
    //assert(res == VK_SUCCESS);

	// Semaphores (Used for correct command ordering)
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;

	auto logic_dev = m_logic_dev->GetHandler();

	// Semaphore used to ensures that image presentation is complete before starting to submit again
	VkResult res = vkCreateSemaphore(logic_dev, &semaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
	assert(res == VK_SUCCESS);

	// Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
	res = vkCreateSemaphore(logic_dev, &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
	assert(res == VK_SUCCESS);

	// Fences (Used to check draw command buffer completion)
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// Create in signaled state so we don't wait on first render of each command buffer
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	waitFences.resize(m_swapchain->GetImageCount());
	for (auto& fence : waitFences)
	{
		VkResult res = vkCreateFence(logic_dev, &fenceCreateInfo, nullptr, &fence);
		assert(res == VK_SUCCESS);
	}

	BuildCommandBuffers();
}

Context::~Context()
{ 
	auto logic_dev = m_logic_dev->GetHandler();

	for (auto& fence : waitFences) {
		vkDestroyFence(logic_dev, fence, nullptr);
	}

	vkDestroySemaphore(logic_dev, presentCompleteSemaphore, nullptr);
	vkDestroySemaphore(logic_dev, renderCompleteSemaphore, nullptr);
}

void Context::Resize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	auto logic_dev = m_logic_dev->GetHandler();

	vkDeviceWaitIdle(logic_dev);

	m_swapchain = std::make_shared<Swapchain>(logic_dev);
	m_swapchain->Create(*this);

	m_depth_buf = std::make_shared<DepthBuffer>(logic_dev);
	m_depth_buf->Create(m_phy_dev->GetHandler(), m_width, m_height);

	m_frame_buffers = std::make_shared<FrameBuffers>(logic_dev);
	m_frame_buffers->Create(*this, m_include_depth);

	m_cmd_bufs = std::make_shared<CommandBuffers>(logic_dev, m_cmd_pool);
	m_cmd_bufs->Create(m_swapchain->GetImageCount());

	vkDeviceWaitIdle(logic_dev);

	BuildCommandBuffers();
}

void Context::Clear(const ClearState& clear_state)
{

}

void Context::Draw(PrimitiveType prim_type, int offset, int count, const DrawState& draw, const void* scene)
{
	Draw();
}

void Context::Draw(PrimitiveType prim_type, const DrawState& draw, const void* scene)
{
	Draw();
}

void Context::SetViewport(int x, int y, int w, int h)
{

}

void Context::GetViewport(int& x, int& y, int& w, int& h) const
{
}

void Context::SetTexture(size_t slot, const ur::TexturePtr& tex)
{

}

void Context::SetTextureSampler(size_t slot, const std::shared_ptr<ur::TextureSampler>& sampler)
{

}

void Context::SetFramebuffer(const std::shared_ptr<ur::Framebuffer>& fb) 
{
}

std::shared_ptr<ur::Framebuffer> Context::GetFramebuffer() const 
{
    return nullptr;
}

void Context::SetUnpackRowLength(int len)
{

}

void Context::SetPackRowLength(int len)
{

}

bool Context::CheckRenderTargetStatus()
{
	return true;
}

void Context::Flush()
{
	vkDeviceWaitIdle(m_logic_dev->GetHandler());
}

void Context::Init(uint32_t width, uint32_t height, void* hwnd)
{
    auto inst = m_dev.GetInstance()->GetHandler();

    m_surface = std::make_shared<Surface>(inst);
    m_surface->Create(hwnd);

    m_phy_dev = std::make_shared<PhysicalDevice>(inst, m_surface->GetHandler());
    m_phy_dev->Create();

    m_logic_dev = std::make_shared<LogicalDevice>();
    m_logic_dev->Create(m_dev.IsEnableValidationLayers(), *m_phy_dev, m_surface->GetHandler());

    auto logic_dev = m_logic_dev->GetHandler();

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
    m_depth_buf->Create(m_phy_dev->GetHandler(), m_width, m_height);

    m_uniform_buf = std::make_shared<UniformBuffer>(logic_dev);
    m_uniform_buf->Create(m_phy_dev->GetHandler(), m_width, m_height);

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
    m_renderpass->Create(m_phy_dev->GetHandler(), m_surface->GetHandler(), *m_depth_buf, m_include_depth);

    m_frame_buffers = std::make_shared<FrameBuffers>(logic_dev);
    m_frame_buffers->Create(*this, m_include_depth);

    m_vert_buf = std::make_shared<vulkan::VertexBuffer>(logic_dev);
    m_vert_buf->Create(m_phy_dev->GetHandler(), g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
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

void Context::Draw()
{
	auto vk_dev = m_logic_dev->GetHandler();

	// Get next image in the swap chain (back/front buffer)
	VkResult res = vkAcquireNextImageKHR(vk_dev, m_swapchain->GetHandler(),
		UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &m_current_buffer);
    assert(res == VK_SUCCESS);

	// Use a fence to wait until the command buffer has finished execution before using it again
	res = vkWaitForFences(vk_dev, 1, &waitFences[m_current_buffer], VK_TRUE, UINT64_MAX);
	assert(res == VK_SUCCESS);
	res = vkResetFences(vk_dev, 1, &waitFences[m_current_buffer]);
	assert(res == VK_SUCCESS);

	// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// The submit info structure specifices a command buffer queue submission batch
	VkSubmitInfo submitInfo = {};
	//submitInfo.pNext = NULL;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitStageMask;               // Pointer to the list of pipeline stages that the semaphore waits will occur at
	submitInfo.pWaitSemaphores = &presentCompleteSemaphore;      // Semaphore(s) to wait upon before the submitted command buffer starts executing
	submitInfo.waitSemaphoreCount = 1;                           // One wait semaphore
	submitInfo.pSignalSemaphores = &renderCompleteSemaphore;     // Semaphore(s) to be signaled when command buffers have completed
	submitInfo.signalSemaphoreCount = 1;                         // One signal semaphore
	submitInfo.pCommandBuffers = &m_cmd_bufs->GetHandler()[m_current_buffer];                // Command buffers(s) to execute in this batch (submission)
	submitInfo.commandBufferCount = 1;                           // One command buffer

	auto graphics_queue = m_logic_dev->GetGraphicsQueue();

	// Submit to the graphics queue passing a wait fence
	res = vkQueueSubmit(graphics_queue, 1, &submitInfo, waitFences[m_current_buffer]);
	assert(res == VK_SUCCESS);

	// Present the current buffer to the swap chain
	// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
	// This ensures that the image is not presented to the windowing system until all commands have been submitted
	VkResult present = m_swapchain->QueuePresent(graphics_queue, m_current_buffer, renderCompleteSemaphore);
	if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {
		assert(present == VK_SUCCESS);
	}
}

void Context::BuildCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = nullptr;

	// Set clear values for all framebuffer attachments with loadOp set to clear
	// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = m_renderpass->GetHandler();
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = m_width;
	renderPassBeginInfo.renderArea.extent.height = m_height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	auto& cmd_bufs = m_cmd_bufs->GetHandler();
	auto& frame_bufs = m_frame_buffers->GetHandler();
	for (size_t i = 0; i < cmd_bufs.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = frame_bufs[i];

		VkResult res = vkBeginCommandBuffer(cmd_bufs[i], &cmdBufInfo);
		assert(res == VK_SUCCESS);

		// Start the first sub pass specified in our default render pass setup by the base class
		// This will clear the color and depth attachment
		vkCmdBeginRenderPass(cmd_bufs[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.width = (float)m_width;
		viewport.height = (float)m_height;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(cmd_bufs[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = m_width;
		scissor.extent.height = m_height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(cmd_bufs[i], 0, 1, &scissor);

		// Bind descriptor sets describing shader binding points
		std::vector<VkDescriptorSet> desc_sets;
		desc_sets.push_back(m_desc_set->GetHandler());
		vkCmdBindDescriptorSets(cmd_bufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout->GetHandler(), 0, desc_sets.size(), desc_sets.data(), 0, nullptr);

		// Bind the rendering pipeline
		// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
		vkCmdBindPipeline(cmd_bufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetHandler());

		// Bind triangle vertex buffer (contains position and colors)
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmd_bufs[i], 0, 1, &m_vert_buf->GetBuffer(), offsets);

		//// Bind triangle index buffer
		//vkCmdBindIndexBuffer(cmd_bufs[i], m_info.idx_buf->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

		//// Draw indexed triangle
		//vkCmdDrawIndexed(cmd_bufs[i], m_info.idx_buf->GetCount(), 1, 0, 0, 1);

		vkCmdDraw(cmd_bufs[i], 12 * 3, 1, 0, 0);

		vkCmdEndRenderPass(cmd_bufs[i]);

		// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

		res = vkEndCommandBuffer(cmd_bufs[i]);
		assert(res == VK_SUCCESS);
	}
}

}
}