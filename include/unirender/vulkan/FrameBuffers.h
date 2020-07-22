#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class VulkanContext;

class FrameBuffers
{
public:
	FrameBuffers(VkDevice device);
	~FrameBuffers();

	void Create(const VulkanContext& vk_ctx, bool include_depth);

	auto& GetHandler() const { return m_frame_buffers; }

private:
	VkDevice m_device = VK_NULL_HANDLE;

	std::vector<VkFramebuffer> m_frame_buffers;

}; // FrameBuffers

}
}