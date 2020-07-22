#pragma once

#include <vulkan/vulkan.h>

#include <optional>

namespace ur
{
namespace vulkan
{

class PhysicalDevice
{
public:
	PhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
	~PhysicalDevice();

	void Create();

	auto GetHandler() const { return m_handle; }

	static uint32_t FindMemoryType(VkPhysicalDevice phy_dev,
		uint32_t type_filter, VkMemoryPropertyFlags properties);

public:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		bool IsComplete() {
			return graphics_family.has_value() && present_family.has_value();
		}
	};

	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

private:
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;

	bool IsDeviceSuitable(VkPhysicalDevice device) const;

private:
	VkInstance m_instance;
	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_handle = VK_NULL_HANDLE;

}; // PhysicalDevice

}
}
