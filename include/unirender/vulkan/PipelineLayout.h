#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class DescriptorSetLayout;

class PipelineLayout : boost::noncopyable
{
public:
	PipelineLayout(const std::shared_ptr<LogicalDevice>& device,
		const std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts);
	~PipelineLayout();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = VK_NULL_HANDLE;

	VkPipelineLayout m_handle = VK_NULL_HANDLE;

}; // PipelineLayout

}
}