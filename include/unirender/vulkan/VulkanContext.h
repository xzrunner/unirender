#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

namespace ur
{
namespace vulkan
{

class VulkanDevice;

class VulkanContext
{
public:
	VulkanContext(const VulkanDevice& vk);
	~VulkanContext();

	auto GetSurface() const { return m_surface; }

	auto GetPhysicalDevice() const { return m_physical_device; }
	auto GetDevice() const { return m_device; }

	auto GetGraphicsQueue() const { return m_graphics_queue; }
	auto GetPresentQueue() const { return m_present_queue; }

	void Init(uint32_t width, uint32_t height, void* hwnd);
	void Resize(uint32_t width, uint32_t height);

	size_t GetSwapchainImageCount() const { return m_swapchain_images.size(); }

	static uint32_t FindMemoryType(VkPhysicalDevice phy_dev,
		uint32_t type_filter, VkMemoryPropertyFlags properties);

private:
	void CreateSurface(void* hwnd);
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain(uint32_t width, uint32_t height);

private:
	struct QueueFamilyIndices 
	{
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		bool IsComplete() {
			return graphics_family.has_value() && present_family.has_value();
		}
	};

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;

	bool IsDeviceSuitable(VkPhysicalDevice device) const;
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;

private:
	const VulkanDevice& m_vk_ctx;

	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	VkDevice m_device;

	VkQueue m_graphics_queue;
	VkQueue m_present_queue;

	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_swapchain_images;
	VkFormat m_swapchain_image_format;
	VkExtent2D m_swapchain_extent;

	// todo
	friend class Swapchain;

}; // VulkanContext

}
}