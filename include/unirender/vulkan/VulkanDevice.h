#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class VulkanDevice
{
public:
	VulkanDevice(bool enable_validation_layers);
	~VulkanDevice();

	bool IsEnableValidationLayers() const { return m_enable_validation_layers; }

	auto GetInstance() const { return m_instance; }

	static const std::vector<const char*>& GetValidationLayers();

private:
	void CreateInstance();
	void SetupDebugMessenger();

	bool CheckValidationLayerSuppor() const;

private:
	bool m_enable_validation_layers = false;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debug_messenger;

	// fixme
public:
	mutable VkDevice m_vk_dev = VK_NULL_HANDLE;

}; // VulkanDevice

}
}