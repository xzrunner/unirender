#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class DeviceInfo;
class ContextInfo;

class Swapchain
{
public:
    Swapchain(VkDevice device);
    ~Swapchain();

    void Create(const DeviceInfo& dev_info, const ContextInfo& ctx_info,
		VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT/* | VK_IMAGE_USAGE_TRANSFER_SRC_BIT*/);

	auto& GetHandler() const { return m_handle; }

	auto GetImageCount() const { return m_image_count; }

	VkImageView GetView(int idx) const { 
		return idx >= 0 && idx < static_cast<int>(m_buffers.size()) ? m_buffers[idx].view : 0;
	}

	VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

private:
	struct Buffer
	{
		VkImage image;
		VkImageView view;
	};

private:
    VkDevice m_device;

    VkSwapchainKHR m_handle = VK_NULL_HANDLE;

	uint32_t m_image_count = 0;
	std::vector<Buffer> m_buffers;

}; // Swapchain

}
}