#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class ContextInfo;

class Swapchain
{
public:
    Swapchain(VkDevice device);
    ~Swapchain();

    void Create(const ContextInfo& ctx_info);

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
    VkDevice m_device = VK_NULL_HANDLE;

    VkSwapchainKHR m_handle = VK_NULL_HANDLE;

	uint32_t m_image_count = 0;
	std::vector<Buffer> m_buffers;

}; // Swapchain

}
}