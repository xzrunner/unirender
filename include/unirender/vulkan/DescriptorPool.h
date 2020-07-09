#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class DescriptorPool
{
public:
	DescriptorPool(VkDevice device);
	~DescriptorPool();

	void Create(bool use_texture);

	auto GetHandler() const { return m_handle; }

private:
	VkDevice m_device;

	VkDescriptorPool m_handle = VK_NULL_HANDLE;

}; // DescriptorPool

}
}