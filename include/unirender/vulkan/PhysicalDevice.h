#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>


namespace ur
{
namespace vulkan
{

class Instance;
class Surface;

class PhysicalDevice : noncopyable
{
public:
	PhysicalDevice(const Instance& instance, const Surface* surface = nullptr);

	auto GetHandler() const { return m_handle; }

	static const std::vector<const char*>& GetDeviceExtensions();

public:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		bool IsComplete(const Surface* surface)
		{
			if (surface) {
				return graphics_family.has_value() && present_family.has_value();
			} else {
				return graphics_family.has_value();
			}
		}
	};

	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, const Surface* surface);

private:
	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	static bool IsDeviceSuitable(VkPhysicalDevice device, const Surface* surface);

private:
	VkPhysicalDevice m_handle = VK_NULL_HANDLE;

}; // PhysicalDevice

}
}
