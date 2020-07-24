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
class DescriptorPool;
class DescriptorSetLayout;

class DescriptorSet : boost::noncopyable
{
public:
	DescriptorSet(const std::shared_ptr<LogicalDevice>& device, 
		const DescriptorPool& pool,
		const std::vector<std::shared_ptr<DescriptorSetLayout>>& layouts,
		const std::vector<VkWriteDescriptorSet>& descriptors);
	~DescriptorSet();

	auto GetHandler() const { return m_handle; }

	static void AddDescriptor(std::vector<VkWriteDescriptorSet>& descriptors,
		VkDescriptorType type, const VkDescriptorBufferInfo* buffer_info);
	static void AddDescriptor(std::vector<VkWriteDescriptorSet>& descriptors,
		VkDescriptorType type, const VkDescriptorImageInfo* image_info);

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkDescriptorSet m_handle = VK_NULL_HANDLE;

	//std::vector<std::shared_ptr<DescriptorSetLayout>> m_layouts;
	//std::vector<VkWriteDescriptorSet> m_descriptors;

}; // DescriptorSet

}
}