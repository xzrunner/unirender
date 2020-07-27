#include "unirender/vulkan/Context.h"
#include "unirender/vulkan/Utility.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/CommandBuffer.h"
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/FrameBuffers.h"
#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/PipelineCache.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/DescriptorPool.h"
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
#include "unirender/vulkan/Texture.h"
#include "unirender/Adaptor.h"
#include "unirender/DrawState.h"
#include "unirender/VertexArray.h"

#include <vulkan/vulkan.h>

#include <iostream>

#include <assert.h>

namespace ur
{
namespace vulkan
{

Context::Context(const ur::Device& device, void* hwnd,
	             uint32_t width, uint32_t height)
	: m_device(static_cast<const vulkan::Device&>(device))
{
	Init(hwnd, width, height);

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

	auto logic_dev = m_device.m_logic_dev->GetHandler();

	// Semaphore used to ensures that image presentation is complete before starting to submit again
	VkResult res = vkCreateSemaphore(logic_dev, &semaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
	assert(res == VK_SUCCESS);

	// Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
	res = vkCreateSemaphore(logic_dev, &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
	assert(res == VK_SUCCESS);

	// Fences (Used to check draw command buffer completion)
	VkFenceCreateInfo fence_ci = {};
	fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// Create in signaled state so we don't wait on first render of each command buffer
	fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	res = vkCreateFence(logic_dev, &fence_ci, nullptr, &m_wait_fence);
	assert(res == VK_SUCCESS);
}

Context::~Context()
{ 
	auto logic_dev = m_device.m_logic_dev->GetHandler();

	vkDestroyFence(logic_dev, m_wait_fence, nullptr);

	vkDestroySemaphore(logic_dev, presentCompleteSemaphore, nullptr);
	vkDestroySemaphore(logic_dev, renderCompleteSemaphore, nullptr);
}

void Context::Resize(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	auto vk_dev = m_device.m_logic_dev->GetHandler();

	vkDeviceWaitIdle(vk_dev);

	// should destroy before create
	m_swapchain.reset();
	m_swapchain = std::make_shared<Swapchain>(m_device.m_logic_dev, *m_device.m_phy_dev, *m_surface, m_width, m_height);

	m_depth_buf = std::make_shared<DepthBuffer>(m_device.m_logic_dev, *m_device.m_phy_dev, m_width, m_height);

	m_frame_buffers = std::make_shared<FrameBuffers>(*this, m_include_depth);

	m_cmd_buf = std::make_shared<CommandBuffer>(m_device.m_logic_dev, m_cmd_pool);
}

void Context::Clear(const ClearState& clear_state)
{

}

void Context::Draw(PrimitiveType prim_type, int offset, int count, const DrawState& draw, const void* scene)
{
	Draw(draw);
}

void Context::Draw(PrimitiveType prim_type, const DrawState& draw, const void* scene)
{
	Draw(draw);
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
	vkDeviceWaitIdle(m_device.m_logic_dev->GetHandler());
}

std::shared_ptr<ur::Pipeline>
Context::CreatePipeline(bool include_depth, bool include_vi, const ur::PipelineLayout& layout,
                        const ur::VertexBuffer& vb, const ur::ShaderProgram& prog) const
{
    return std::make_shared<Pipeline>(*this, m_include_depth, include_vi, layout, vb, prog);
}

void Context::Init(void* hwnd, uint32_t width, uint32_t height)
{
    m_surface = std::make_shared<Surface>(m_device.m_instance, hwnd);

    auto phy_dev = std::make_shared<PhysicalDevice>(*m_device.m_instance, m_surface.get());
	if (phy_dev->GetHandler() != m_device.m_phy_dev->GetHandler()) {
		throw std::runtime_error("different physical device!");
	}

	PhysicalDevice::QueueFamilyIndices indices = PhysicalDevice::FindQueueFamilies(phy_dev->GetHandler(), m_surface.get());

	if (!m_device.m_logic_dev) {
		const_cast<Device&>(m_device).m_logic_dev = std::make_shared<LogicalDevice>(m_device.m_enable_validation_layers, *m_device.m_phy_dev, m_surface.get());
		const_cast<Device&>(m_device).m_present_family_id = indices.present_family.value();
	} else {
		if (indices.present_family.value() != m_device.m_present_family_id) {
			throw std::runtime_error("different logic device!");
		}
	}

    m_width = width;
    m_height = height;

	const bool use_texture = false;
	const bool include_vi = true;

    m_swapchain = std::make_shared<Swapchain>(m_device.m_logic_dev, *m_device.m_phy_dev, *m_surface, m_width, m_height);

    m_cmd_pool = std::make_shared<CommandPool>(m_device.m_logic_dev);
    m_cmd_buf = std::make_shared<CommandBuffer>(m_device.m_logic_dev, m_cmd_pool);

    m_depth_buf = std::make_shared<DepthBuffer>(m_device.m_logic_dev, *m_device.m_phy_dev, m_width, m_height);

	std::vector<std::pair<DescriptorType, ShaderType>> single_ubo_bindings = {
		{ ur::DescriptorType::UniformBuffer, ur::ShaderType::VertexShader } 
	};
	const_cast<Device&>(m_device).SetDescriptorSetLayout("single_ubo", m_device.CreateDescriptorSetLayout(single_ubo_bindings));

	std::vector<std::pair<DescriptorType, ShaderType>> single_img_bindings = {
		{ ur::DescriptorType::CombinedImageSampler, ur::ShaderType::FragmentShader }
	};
	const_cast<Device&>(m_device).SetDescriptorSetLayout("single_img", m_device.CreateDescriptorSetLayout(single_img_bindings));

	std::vector<std::shared_ptr<ur::DescriptorSetLayout>> layouts = { m_device.GetDescriptorSetLayout("single_ubo") };
	//if (use_texture) {
	//	layouts.push_back(m_device.GetDescriptorSetLayout("single_img"));
	//}
	const_cast<Device&>(m_device).SetPipelineLayout("single_img", std::make_shared<PipelineLayout>(m_device.m_logic_dev, layouts));

    m_renderpass = std::make_shared<RenderPass>(*this, m_include_depth);

    m_frame_buffers = std::make_shared<FrameBuffers>(*this, m_include_depth);

	std::vector<std::pair<ur::DescriptorType, size_t>> pool_sizes = {
		{ ur::DescriptorType::UniformBuffer,        1024 },
		{ ur::DescriptorType::CombinedImageSampler, 1024 },
	};
	const_cast<Device&>(m_device).SetDescriptorPool(m_device.CreateDescriptorPool(1024, pool_sizes));

    m_pipeline_cache = std::make_shared<PipelineCache>(m_device.m_logic_dev);
}

std::shared_ptr<PhysicalDevice> Context::GetPhysicalDevice() const
{ 
	return m_device.m_phy_dev; 
}

std::shared_ptr<LogicalDevice> Context::GetLogicalDevice() const
{ 
	return m_device.m_logic_dev; 
}

void Context::Draw(const DrawState& draw)
{
	auto vk_dev = m_device.m_logic_dev->GetHandler();

	// Get next image in the swap chain (back/front buffer)
	VkResult res = vkAcquireNextImageKHR(vk_dev, m_swapchain->GetHandler(),
		UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &m_current_buffer);
    assert(res == VK_SUCCESS);

	// Use a fence to wait until the command buffer has finished execution before using it again
	res = vkWaitForFences(vk_dev, 1, &m_wait_fence, VK_TRUE, UINT64_MAX);
	assert(res == VK_SUCCESS);
	res = vkResetFences(vk_dev, 1, &m_wait_fence);
	assert(res == VK_SUCCESS);

	BuildCommandBuffers(draw);

	// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// The submit info structure specifices a command buffer queue submission batch
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pWaitDstStageMask = &waitStageMask;               // Pointer to the list of pipeline stages that the semaphore waits will occur at
	submit_info.pWaitSemaphores = &presentCompleteSemaphore;      // Semaphore(s) to wait upon before the submitted command buffer starts executing
	submit_info.waitSemaphoreCount = 1;                           // One wait semaphore
	submit_info.pSignalSemaphores = &renderCompleteSemaphore;     // Semaphore(s) to be signaled when command buffers have completed
	submit_info.signalSemaphoreCount = 1;                         // One signal semaphore
	auto cmd_buf = m_cmd_buf->GetHandler();
	submit_info.pCommandBuffers = &cmd_buf;                       // Command buffers(s) to execute in this batch (submission)
	submit_info.commandBufferCount = 1;                           // One command buffer

	auto graphics_queue = m_device.m_logic_dev->GetGraphicsQueue();

	// Submit to the graphics queue passing a wait fence

	//// Create fence to ensure that the command buffer has finished executing
	//VkFenceCreateInfo fence_ci = {};
	//fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	//fence_ci.flags = 0;
	//VkFence fence;
	//res = vkCreateFence(vk_dev, &fence_ci, nullptr, &fence);
	//assert(res == VK_SUCCESS);

	res = vkQueueSubmit(graphics_queue, 1, &submit_info, m_wait_fence);
	assert(res == VK_SUCCESS);

	//res = vkWaitForFences(vk_dev, 1, &fence, VK_TRUE, UINT64_MAX);
	//assert(res == VK_SUCCESS);
	//vkDestroyFence(vk_dev, fence, nullptr);

	// Present the current buffer to the swap chain
	// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
	// This ensures that the image is not presented to the windowing system until all commands have been submitted
	VkResult present = m_swapchain->QueuePresent(graphics_queue, m_current_buffer, renderCompleteSemaphore);
	if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {
		assert(present == VK_SUCCESS);
	}
}

void Context::BuildCommandBuffers(const DrawState& draw)
{
	VkResult res;

	auto vk_dev = m_device.m_logic_dev->GetHandler();

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

	auto cmd_buf = m_cmd_buf->GetHandler();
	auto& frame_bufs = m_frame_buffers->GetHandler();

	// Set target frame buffer
	renderPassBeginInfo.framebuffer = frame_bufs[m_current_buffer];

	res = vkBeginCommandBuffer(cmd_buf, &cmdBufInfo);
	assert(res == VK_SUCCESS);

	// Start the first sub pass specified in our default render pass setup by the base class
	// This will clear the color and depth attachment
	vkCmdBeginRenderPass(cmd_buf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Update dynamic viewport state
	VkViewport viewport = {};
	viewport.width = (float)m_width;
	viewport.height = (float)m_height;
	viewport.minDepth = (float) 0.0f;
	viewport.maxDepth = (float) 1.0f;
	vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

	// Update dynamic scissor state
	VkRect2D scissor = {};
	scissor.extent.width = m_width;
	scissor.extent.height = m_height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

	// Bind descriptor sets describing shader binding points
	std::vector<VkDescriptorSet> desc_sets;
	desc_sets.push_back(std::static_pointer_cast<vulkan::DescriptorSet>(draw.desc_set)->GetHandler());
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		std::static_pointer_cast<vulkan::PipelineLayout>(draw.pipeline_layout)->GetHandler(), 0, desc_sets.size(), desc_sets.data(), 0, nullptr);

	// Bind the rendering pipeline
	// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		std::static_pointer_cast<vulkan::Pipeline>(draw.pipeline)->GetHandler());

	// Bind triangle vertex buffer (contains position and colors)
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(cmd_buf, 0, 1, &std::static_pointer_cast<vulkan::VertexBuffer>(draw.vertex_array->GetVertexBuffer())->GetBuffer(), offsets);

	//// Bind triangle index buffer
	//vkCmdBindIndexBuffer(cmd_buf, m_info.idx_buf->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

	//// Draw indexed triangle
	//vkCmdDrawIndexed(cmd_buf, m_info.idx_buf->GetCount(), 1, 0, 0, 1);

	auto ib = draw.vertex_array->GetIndexBuffer();
	auto vb = draw.vertex_array->GetVertexBuffer();
	if (ib)
	{

	}
	else
	{
		vkCmdDraw(cmd_buf, vb->GetVertexCount(), 1, 0, 0);
	}

	vkCmdEndRenderPass(cmd_buf);

	// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
	// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

	res = vkEndCommandBuffer(cmd_buf);
	assert(res == VK_SUCCESS);
}

}
}