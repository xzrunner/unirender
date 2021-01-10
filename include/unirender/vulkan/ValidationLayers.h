#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>


namespace ur
{
namespace vulkan
{

class Instance;

class ValidationLayers : noncopyable
{
public:
	ValidationLayers(const std::shared_ptr<Instance>& instance);
	~ValidationLayers();

	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static const std::vector<const char*>& GetValidationLayers();

	static bool CheckValidationLayerSuppor();

private:
	std::shared_ptr<Instance> m_instance = nullptr;

	VkDebugUtilsMessengerEXT m_debug_messenger;

}; // ValidationLayers

}
}