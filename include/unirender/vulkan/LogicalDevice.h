#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class VulkanDevice;
class Surface;
class PhysicalDevice;

class LogicalDevice
{
public:
	LogicalDevice();
	~LogicalDevice();

	void Create(const VulkanDevice& vk_dev, const Surface& surface, 
		const PhysicalDevice& phy_dev);

	auto GetHandler() const { return m_handle; }

	auto GetGraphicsQueue() const { return m_graphics_queue; }
	auto GetPresentQueue() const { return m_present_queue; }

private:
	VkDevice m_handle = VK_NULL_HANDLE;

	VkQueue m_graphics_queue;
	VkQueue m_present_queue;

}; // LogicalDevice

}
}