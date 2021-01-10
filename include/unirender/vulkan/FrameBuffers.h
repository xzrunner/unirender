#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>


namespace ur
{
namespace vulkan
{

class Context;
class LogicalDevice;

class FrameBuffers : noncopyable
{
public:
	FrameBuffers(const Context& ctx, bool include_depth);
	~FrameBuffers();

	auto& GetHandler() const { return m_frame_buffers; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	std::vector<VkFramebuffer> m_frame_buffers;

}; // FrameBuffers

}
}