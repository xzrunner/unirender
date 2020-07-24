#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class Context;
class LogicalDevice;

class FrameBuffers : boost::noncopyable
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