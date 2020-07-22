#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class ContextInfo;

class FrameBuffers
{
public:
	FrameBuffers(VkDevice device);
	~FrameBuffers();

	void Create(const ContextInfo& ctx_info, bool include_depth);

	auto& GetHandle() const { return m_frame_buffers; }

private:
	VkDevice m_device = VK_NULL_HANDLE;

	std::vector<VkFramebuffer> m_frame_buffers;

}; // FrameBuffers

}
}