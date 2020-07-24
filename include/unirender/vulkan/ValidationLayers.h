#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class Instance;

class ValidationLayers : boost::noncopyable
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