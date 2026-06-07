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
		VkImageType type, VkFormat format, VkExtent3D extent, uint32_t mip_levels = 1, uint32_t array_layers = 1,
		bool as_color_attachment = false);
	~Image();

	auto GetHandler() const { return m_handle; }

	void Upload(const PhysicalDevice& phy_dev, const void* data, size_t size);

	// Upload a sub-region (x,y,w,h) of tightly-packed pixels into an image that is
	// currently in SHADER_READ_ONLY_OPTIMAL (a sampled / color-attachment texture),
	// preserving the rest of the image. Used by Texture::Upload for incremental
	// updates, e.g. dtex packing glyphs into the GUI atlas.
	void UploadRegion(const PhysicalDevice& phy_dev, const void* data, size_t size,
		uint32_t x, uint32_t y, uint32_t w, uint32_t h);

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

	VkExtent3D m_extent = { 0, 0, 1 }; // for Upload's buffer->image copy

}; // Image

}
}