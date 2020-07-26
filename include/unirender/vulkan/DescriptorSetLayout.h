#pragma once

#include "unirender/DescriptorType.h"
#include "unirender/ShaderType.h"
#include "unirender/DescriptorSetLayout.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

namespace ur
{
namespace vulkan
{

class LogicalDevice;

class DescriptorSetLayout : public ur::DescriptorSetLayout
{
public:
	DescriptorSetLayout(const std::shared_ptr<LogicalDevice>& device,
		const std::vector<std::pair<DescriptorType, ShaderType>>& bindings);
	~DescriptorSetLayout();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;

}; // DescriptorSetLayout

}
}