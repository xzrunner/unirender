#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

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

	void Create();

	void AddLayout(const std::shared_ptr<DescriptorSetLayout>& layout) {
		m_layouts.push_back(layout);
	}

	auto GetHandler() const { return m_handle; }

private:
	VkDevice m_device = VK_NULL_HANDLE;

	VkPipelineLayout m_handle = VK_NULL_HANDLE;

	std::vector<std::shared_ptr<DescriptorSetLayout>> m_layouts;

}; // PipelineLayout

}
}