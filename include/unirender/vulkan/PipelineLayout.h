#pragma once

#include "unirender/PipelineLayout.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

namespace ur
{

class DescriptorSetLayout;

namespace vulkan
{

class LogicalDevice;

class PipelineLayout : public ur::PipelineLayout
{
public:
	PipelineLayout(const std::shared_ptr<LogicalDevice>& device,
		const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& layouts);
	~PipelineLayout();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = VK_NULL_HANDLE;

	VkPipelineLayout m_handle = VK_NULL_HANDLE;

}; // PipelineLayout

}
}