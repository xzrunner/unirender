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

	void Create(bool use_texture, VkDescriptorSetLayoutCreateFlags descSetLayoutCreateFlags = 0);

	auto& GetHandler() const { return m_handle; }

private:
	VkDevice m_device;

	std::vector<VkDescriptorSetLayout> m_handle;

}; // DescriptorSetLayout

}
}