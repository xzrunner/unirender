#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class DescriptorSetLayout;

class PipelineLayout
{
public:
	PipelineLayout(VkDevice device);
	~PipelineLayout();

	void Create(const DescriptorSetLayout& desc_set_layout);

	auto GetHandler() const { return m_handle; }

private:
	VkDevice m_device;

	VkPipelineLayout m_handle = VK_NULL_HANDLE;

}; // PipelineLayout

}
}