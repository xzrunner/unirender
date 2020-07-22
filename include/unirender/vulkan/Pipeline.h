#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class VulkanContext;

class Pipeline
{
public:
	Pipeline(VkDevice device);
	~Pipeline();

	void Create(const VulkanContext& vk_ctx,
		bool include_depth, bool include_vi);

	auto GetHandler() const { return m_handle; }

private:
	VkDevice m_device = VK_NULL_HANDLE;

	VkPipeline m_handle;

}; // Pipeline

}
}