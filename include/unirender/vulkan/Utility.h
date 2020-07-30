#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class Utility
{
public:
	static void CreateBuffer(VkDevice device, VkPhysicalDevice phy_dev, VkDeviceSize size,
		VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);

	static uint32_t FindMemoryType(VkPhysicalDevice phy_dev,
		uint32_t type_filter, VkMemoryPropertyFlags properties);

}; // Utility

}
}