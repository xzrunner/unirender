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

class DescriptorSetLayout : boost::noncopyable
{
public:
	DescriptorSetLayout(const std::shared_ptr<LogicalDevice>& device,
		const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	~DescriptorSetLayout();

	auto GetHandler() const { return m_handle; }

	static void AddBinding(std::vector<VkDescriptorSetLayoutBinding>& bindings,
		VkDescriptorType desc_type, VkShaderStageFlags stage_flags);

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;

}; // DescriptorSetLayout

}
}