#include "unirender/vulkan/Context.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/Utility.h"
#include "unirender/vulkan/CommandBuffers.h"
#include "unirender/vulkan/DeviceInfo.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/FrameBuffers.h"
#include "unirender/vulkan/Pipeline.h"
#include "unirender/vulkan/PipelineLayout.h"
#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/IndexBuffer.h"

#include <vulkan/vulkan.h>

#include <iostream>

#include <assert.h>
#include "../../../gtxt/src/gtxt/gtxt_label.h"

#undef DrawState

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}

namespace
{

VkSemaphore presentCompleteSemaphore;
VkSemaphore renderCompleteSemaphore;

//VkSemaphore imageAcquiredSemaphore;

std::vector<VkFence> waitFences;

}

namespace ur
{
namespace vulkan
{

Context::Context(const ur::Device& device, void* hwnd,
	             uint32_t width, uint32_t height)
    : m_dev_info(static_cast<const vulkan::Device&>(device).GetInfo())
	, m_info(m_dev_info, false)
{
	m_info.Init(width, height, hwnd);

    //VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
    //imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    //imageAcquiredSemaphoreCreateInfo.pNext = NULL;
    //imageAcquiredSemaphoreCreateInfo.flags = 0;

    //VkResult res = vkCreateSemaphore(m_dev_info.device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
    //assert(res == VK_SUCCESS);

	// Semaphores (Used for correct command ordering)
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;

	// Semaphore used to ensures that image presentation is complete before starting to submit again
	VkResult res = vkCreateSemaphore(m_dev_info.device, &semaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
	assert(res == VK_SUCCESS);

	// Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
	res = vkCreateSemaphore(m_dev_info.device, &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
	assert(res == VK_SUCCESS);

	// Fences (Used to check draw command buffer completion)
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// Create in signaled state so we don't wait on first render of each command buffer
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	waitFences.resize(m_info.cmd_bufs->GetHandler().size());
	for (auto& fence : waitFences)
	{
		VkResult res = vkCreateFence(m_dev_info.device, &fenceCreateInfo, nullptr, &fence);
		assert(res == VK_SUCCESS);
	}

	BuildCommandBuffers();
}

Context::~Context()
{ 
	for (auto& fence : waitFences) {
		vkDestroyFence(m_dev_info.device, fence, nullptr);
	}

	vkDestroySemaphore(m_dev_info.device, presentCompleteSemaphore, nullptr);
	vkDestroySemaphore(m_dev_info.device, renderCompleteSemaphore, nullptr);
}

void Context::Resize(uint32_t width, uint32_t height)
{
	m_info.Resize(width, height);

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
	vkDeviceWaitIdle(m_dev_info.device);
}

void Context::Draw()
{
	// Get next image in the swap chain (back/front buffer)
	VkResult res = vkAcquireNextImageKHR(m_dev_info.device, m_info.swapchain->GetHandler(), 
		UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &m_info.current_buffer);
    assert(res == VK_SUCCESS);

	// Use a fence to wait until the command buffer has finished execution before using it again
	res = vkWaitForFences(m_dev_info.device, 1, &waitFences[m_info.current_buffer], VK_TRUE, UINT64_MAX);
	assert(res == VK_SUCCESS);
	res = vkResetFences(m_dev_info.device, 1, &waitFences[m_info.current_buffer]);
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
	submitInfo.pCommandBuffers = &m_info.cmd_bufs->GetHandler()[m_info.current_buffer];                // Command buffers(s) to execute in this batch (submission)
	submitInfo.commandBufferCount = 1;                           // One command buffer

	// Submit to the graphics queue passing a wait fence
	res = vkQueueSubmit(m_dev_info.graphics_queue, 1, &submitInfo, waitFences[m_info.current_buffer]);
	assert(res == VK_SUCCESS);

	// Present the current buffer to the swap chain
	// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
	// This ensures that the image is not presented to the windowing system until all commands have been submitted
	VkResult present = m_info.swapchain->QueuePresent(m_dev_info.graphics_queue, m_info.current_buffer, renderCompleteSemaphore);
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
	renderPassBeginInfo.renderPass = m_info.renderpass->GetHandler();
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = m_info.width;
	renderPassBeginInfo.renderArea.extent.height = m_info.height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	auto& cmd_bufs = m_info.cmd_bufs->GetHandler();
	for (int32_t i = 0; i < cmd_bufs.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = m_info.frame_buffers->GetHandle()[i];

		VkResult res = vkBeginCommandBuffer(cmd_bufs[i], &cmdBufInfo);
		assert(res == VK_SUCCESS);

		// Start the first sub pass specified in our default render pass setup by the base class
		// This will clear the color and depth attachment
		vkCmdBeginRenderPass(cmd_bufs[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.height = (float)m_info.height;
		viewport.width = (float)m_info.width;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(cmd_bufs[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = m_info.width;
		scissor.extent.height = m_info.height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(cmd_bufs[i], 0, 1, &scissor);

		// Bind descriptor sets describing shader binding points
		vkCmdBindDescriptorSets(cmd_bufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_info.pipeline_layout->GetHandler(), 0, 1, m_info.desc_set->GetHandler().data(), 0, nullptr);

		// Bind the rendering pipeline
		// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
		vkCmdBindPipeline(cmd_bufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_info.pipeline->GetHandler());

		// Bind triangle vertex buffer (contains position and colors)
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmd_bufs[i], 0, 1, &m_info.vert_buf->GetBuffer(), offsets);

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