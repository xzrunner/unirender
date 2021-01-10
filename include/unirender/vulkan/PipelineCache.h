#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>


namespace ur
{
namespace vulkan
{

class LogicalDevice;

class PipelineCache : noncopyable
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