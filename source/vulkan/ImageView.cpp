#include "unirender/vulkan/ImageView.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Image.h"

#include <stdexcept>

namespace ur
{
namespace vulkan
{

ImageView::ImageView(const std::shared_ptr<LogicalDevice>& device, VkImageViewType type,
	                 VkFormat format, VkImageSubresourceRange range, const Image& image)
	: m_device(device)
{
	VkImageViewCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ci.viewType         = type;
	ci.format           = format;
	ci.subresourceRange = range;
	ci.image            = image.GetHandler();
	if (vkCreateImageView(m_device->GetHandler(), &ci, nullptr, &m_handle) != VK_SUCCESS){
		throw std::runtime_error("failed to create image view!");
	}
}

ImageView::~ImageView()
{
	vkDestroyImageView(m_device->GetHandler(), m_handle, nullptr);
}

}
}