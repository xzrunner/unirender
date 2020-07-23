#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class ValidationLayers
{
public:
	ValidationLayers(VkInstance instance);
	~ValidationLayers();

	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static const std::vector<const char*>& GetValidationLayers();

	static bool CheckValidationLayerSuppor();

private:
	VkInstance m_instance;

	VkDebugUtilsMessengerEXT m_debug_messenger;

}; // ValidationLayers

}
}