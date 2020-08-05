#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class Utility
{
public:
	static uint32_t FindMemoryType(VkPhysicalDevice phy_dev,
		uint32_t type_filter, VkMemoryPropertyFlags properties);

}; // Utility

}
}