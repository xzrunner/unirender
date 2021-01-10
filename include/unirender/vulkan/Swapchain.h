#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>


namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;
class Surface;

class Swapchain : noncopyable
{
public:
    Swapchain(const std::shared_ptr<LogicalDevice>& device, const PhysicalDevice& phy_dev,
		const Surface& surface, int width, int height);
    ~Swapchain();

	auto& GetHandler() const { return m_handle; }

	auto GetImageCount() const { return m_image_count; }

	VkImageView GetView(int idx) const { 
		return idx >= 0 && idx < static_cast<int>(m_buffers.size()) ? m_buffers[idx].view : 0;
	}

	VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

public:
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};
	
	static SwapChainSupportDetails 
		QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height);

private:
	struct Buffer
	{
		VkImage image;
		VkImageView view;
	};

private:
	std::shared_ptr<LogicalDevice> m_device = VK_NULL_HANDLE;

    VkSwapchainKHR m_handle = VK_NULL_HANDLE;

	uint32_t m_image_count = 0;
	std::vector<Buffer> m_buffers;

}; // Swapchain

}
}