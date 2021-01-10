#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>


namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;
class CommandPool;

class Image : noncopyable
{
public:
	Image(const std::shared_ptr<LogicalDevice>& device, const PhysicalDevice& phy_dev, const std::shared_ptr<CommandPool>& cmd_pool,
		VkImageType type, VkFormat format, VkExtent3D extent, uint32_t mip_levels = 1, uint32_t array_layers = 1);
	~Image();

	auto GetHandler() const { return m_handle; }

	void Upload(const PhysicalDevice& phy_dev, const void* data, size_t size);

private:
	void Upload2(const PhysicalDevice& phy_dev, const void* data, size_t size);

	static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, 
		VkImageLayout new_layout, VkDevice device, VkCommandPool cmd_pool, VkQueue graphics_queue);
	static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
		VkDevice device, VkCommandPool cmd_pool, VkQueue graphics_queue);

private:
	std::shared_ptr<LogicalDevice> m_device   = nullptr;
	std::shared_ptr<CommandPool>   m_cmd_pool = nullptr;

	VkImage m_handle;

	VkDeviceMemory m_memory;

}; // Image

}
}