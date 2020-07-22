#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

namespace ur
{
namespace vulkan
{

class DescriptorPool;
class DescriptorSetLayout;

class DescriptorSet
{
public:
	DescriptorSet(VkDevice device);
	~DescriptorSet();

	void Create(const DescriptorPool& pool);

	void AddLayout(const std::shared_ptr<DescriptorSetLayout>& layout) {
		m_layouts.push_back(layout);
	}

	void AddDescriptor(VkDescriptorType type, const VkDescriptorBufferInfo* buffer_info);
	void AddDescriptor(VkDescriptorType type, const VkDescriptorImageInfo* image_info);

	auto& GetHandler() const { return m_handle; }

private:
	VkDevice m_device = VK_NULL_HANDLE;

	VkDescriptorSet m_handle = VK_NULL_HANDLE;

	std::vector<std::shared_ptr<DescriptorSetLayout>> m_layouts;
	std::vector<VkWriteDescriptorSet> m_descriptors;

}; // DescriptorSet

}
}