#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class DescriptorSetLayout
{
public:
	DescriptorSetLayout(VkDevice device);
	~DescriptorSetLayout();

	void Create();

	void AddBinding(VkDescriptorType desc_type, VkShaderStageFlags stage_flags);

	auto GetHandler() const { return m_handle; }

private:
	VkDevice m_device = VK_NULL_HANDLE;

	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;

	std::vector<VkDescriptorSetLayoutBinding> m_bindings;

}; // DescriptorSetLayout

}
}