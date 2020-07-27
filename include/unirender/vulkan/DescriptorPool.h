#pragma once

#include "unirender/DescriptorPool.h"
#include "unirender/DescriptorType.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace ur
{
namespace vulkan
{

class LogicalDevice;

class DescriptorPool : public ur::DescriptorPool
{
public:
	DescriptorPool(const std::shared_ptr<LogicalDevice>& device, size_t max_sets, 
		const std::vector<std::pair<DescriptorType, size_t>>& pool_sizes);
	~DescriptorPool();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkDescriptorPool m_handle = VK_NULL_HANDLE;

}; // DescriptorPool

}
}