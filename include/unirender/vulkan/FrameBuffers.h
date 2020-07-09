#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class DeviceInfo;
class ContextInfo;

class FrameBuffers
{
public:
	FrameBuffers(VkDevice device);
	~FrameBuffers();

	void Create(const DeviceInfo& dev_info, 
		const ContextInfo& ctx_info, bool include_depth);

	auto& GetHandle() const { return m_frame_buffers; }

private:
	VkDevice m_device;

	std::vector<VkFramebuffer> m_frame_buffers;

}; // FrameBuffers

}
}