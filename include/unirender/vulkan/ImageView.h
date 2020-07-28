#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class Image;

class ImageView : boost::noncopyable
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