#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class PipelineCache
{
public:
	PipelineCache(VkDevice device);
	~PipelineCache();

	void Create();

	auto GetHandler() const { return m_handle; }

private:
	VkDevice m_device = VK_NULL_HANDLE;

	VkPipelineCache m_handle;

}; // PipelineCache

}
}