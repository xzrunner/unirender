#include "unirender/vulkan/Image.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/PhysicalDevice.h"

#include <stdexcept>

namespace ur
{
namespace vulkan
{

Image::Image(const std::shared_ptr<LogicalDevice>& device, const PhysicalDevice& phy_dev,
	         VkImageType type, VkFormat format, VkExtent3D extent, uint32_t mip_levels, uint32_t array_layers)
	: m_device(device)
{
	auto vk_dev = device->GetHandler();

	VkImageCreateInfo img_ci = {};
	img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_ci.imageType   = type;
	img_ci.format      = format;
	img_ci.extent      = extent;
	img_ci.mipLevels   = mip_levels;
	img_ci.arrayLayers = array_layers;
	img_ci.samples     = VK_SAMPLE_COUNT_1_BIT;
	img_ci.tiling      = VK_IMAGE_TILING_OPTIMAL;
	img_ci.usage       = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (vkCreateImage(vk_dev, &img_ci, nullptr, &m_handle) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}
	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(vk_dev, m_handle, &mem_reqs);
	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.allocationSize = mem_reqs.size;
	mem_alloc.memoryTypeIndex = PhysicalDevice::FindMemoryType(
		phy_dev.GetHandler(), mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	if (vkAllocateMemory(vk_dev, &mem_alloc, nullptr, &m_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to alloc image memory!");
	}
	if (vkBindImageMemory(vk_dev, m_handle, m_memory, 0) != VK_SUCCESS) {
		throw std::runtime_error("failed to bind image memory!");
	}
}

Image::~Image()
{
	vkFreeMemory(m_device->GetHandler(), m_memory, nullptr);
	vkDestroyImage(m_device->GetHandler(), m_handle, nullptr);
}

}
}