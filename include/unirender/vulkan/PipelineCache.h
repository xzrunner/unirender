#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;

class PipelineCache : boost::noncopyable
{
public:
	PipelineCache(const std::shared_ptr<LogicalDevice>& device);
	~PipelineCache();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = VK_NULL_HANDLE;

	VkPipelineCache m_handle;

}; // PipelineCache

}
}