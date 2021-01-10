#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>


namespace ur
{
namespace vulkan
{

class LogicalDevice;
class Image;

class ImageView : noncopyable
{
public:
	ImageView(const std::shared_ptr<LogicalDevice>& device, VkImageViewType type, 
		VkFormat format, VkImageSubresourceRange range, const Image& image);
	~ImageView();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkImageView m_handle;

}; // ImageView

}
}