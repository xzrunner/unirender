#include "unirender/vulkan/Image.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/CommandBuffer.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/Buffer.h"
#include "unirender/vulkan/Utility.h"

#include <stdexcept>

namespace ur
{
namespace vulkan
{

Image::Image(const std::shared_ptr<LogicalDevice>& device, const PhysicalDevice& phy_dev, const std::shared_ptr<CommandPool>& cmd_pool,
	         VkImageType type, VkFormat format, VkExtent3D extent, uint32_t mip_levels, uint32_t array_layers,
	         bool as_color_attachment)
	: m_device(device)
	, m_cmd_pool(cmd_pool)
	, m_extent(extent)
{
	auto vk_dev = device->GetHandler();

	// A render target needs OPTIMAL tiling + device-local memory + COLOR_ATTACHMENT
	// usage (plus SAMPLED to read the result back, TRANSFER_DST for the one-time
	// clear). The plain texture path keeps the old LINEAR host-visible behaviour.
	VkImageCreateInfo img_ci = {};
	img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_ci.imageType   = type;
	img_ci.format      = format;
	img_ci.extent      = extent;
	img_ci.mipLevels   = mip_levels;
	img_ci.arrayLayers = array_layers;
	img_ci.samples     = VK_SAMPLE_COUNT_1_BIT;
	img_ci.tiling      = as_color_attachment ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR;
	img_ci.usage       = as_color_attachment
		? (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		: (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	img_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	img_ci.initialLayout = as_color_attachment ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PREINITIALIZED;
	if (vkCreateImage(vk_dev, &img_ci, nullptr, &m_handle) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}
	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(vk_dev, m_handle, &mem_reqs);
	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.allocationSize = mem_reqs.size;
	mem_alloc.memoryTypeIndex = Utility::FindMemoryType(
		phy_dev.GetHandler(), mem_reqs.memoryTypeBits, as_color_attachment
			? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			: (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
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

void Image::Upload(const PhysicalDevice& phy_dev, const void* data, size_t size)
{
	Buffer buffer(m_device);
	buffer.Create(phy_dev.GetHandler(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	buffer.Upload(data, size);

    TransitionImageLayout(m_handle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        m_device->GetHandler(), m_cmd_pool->GetHandler(), m_device->GetGraphicsQueue());
    CopyBufferToImage(buffer.GetHandler(), m_handle, m_extent.width, m_extent.height,
		m_device->GetHandler(), m_cmd_pool->GetHandler(), m_device->GetGraphicsQueue());
    TransitionImageLayout(m_handle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        m_device->GetHandler(), m_cmd_pool->GetHandler(), m_device->GetGraphicsQueue());
}

void Image::UploadRegion(const PhysicalDevice& phy_dev, const void* data, size_t size,
	uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	// Stage the pixels in a host-visible buffer, then copy into the image sub-region.
	Buffer staging(m_device);
	staging.Create(phy_dev.GetHandler(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	staging.Upload(data, size);

	auto dev   = m_device->GetHandler();
	auto pool  = m_cmd_pool->GetHandler();
	auto queue = m_device->GetGraphicsQueue();

	// One command buffer: SHADER_READ_ONLY -> TRANSFER_DST (preserve), copy, -> SHADER_READ_ONLY.
	VkCommandBuffer cb = CommandBuffer::BeginSingleTimeCommands(dev, pool);

	VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	VkImageMemoryBarrier to_dst = {};
	to_dst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	to_dst.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	to_dst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	to_dst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	to_dst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	to_dst.image = m_handle;
	to_dst.subresourceRange = range;
	to_dst.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	to_dst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &to_dst);

	VkBufferImageCopy region = {};
	region.bufferOffset      = 0;
	region.bufferRowLength   = 0; // tightly packed rows of width w
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel       = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount     = 1;
	region.imageOffset = { static_cast<int32_t>(x), static_cast<int32_t>(y), 0 };
	region.imageExtent = { w, h, 1 };
	vkCmdCopyBufferToImage(cb, staging.GetHandler(), m_handle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	VkImageMemoryBarrier to_read = to_dst;
	to_read.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	to_read.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	to_read.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	to_read.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &to_read);

	CommandBuffer::EndSingleTimeCommands(cb, dev, pool, queue);
}

void Image::Upload2(const PhysicalDevice& phy_dev, const void* data, size_t size)
{
	auto vk_dev = m_device->GetHandler();

	// Map image memory
	void* buf;
	if (vkMapMemory(vk_dev, m_memory, 0, size, 0, &buf) != VK_SUCCESS) {
		throw std::runtime_error("failed to map memory!");
	}
	// Copy image data of the first mip level into memory
	memcpy(buf, data, size);
	vkUnmapMemory(vk_dev, m_memory);

	// Setup image memory barrier transfer image to shader read layout
	VkCommandBuffer copyCmd = CommandBuffer::BeginSingleTimeCommands(vk_dev, m_cmd_pool->GetHandler());

	// The sub resource range describes the regions of the image we will be transition
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;

	// Transition the texture image layout to shader read, so it can be sampled from
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = m_handle;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
	// Source pipeline stage is host write/read exection (VK_PIPELINE_STAGE_HOST_BIT)
	// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
	vkCmdPipelineBarrier(
		copyCmd,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);

	CommandBuffer::EndSingleTimeCommands(copyCmd, vk_dev, m_cmd_pool->GetHandler(), m_device->GetGraphicsQueue());
}

void Image::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout,
                                  VkDevice device, VkCommandPool cmd_pool, VkQueue graphics_queue)
{
    VkCommandBuffer cb = CommandBuffer::BeginSingleTimeCommands(device, cmd_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        cb,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    CommandBuffer::EndSingleTimeCommands(cb, device, cmd_pool, graphics_queue);
}

void Image::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                              VkDevice device, VkCommandPool cmd_pool, VkQueue graphics_queue)
{
    VkCommandBuffer cb = CommandBuffer::BeginSingleTimeCommands(device, cmd_pool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(cb, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    CommandBuffer::EndSingleTimeCommands(cb, device, cmd_pool, graphics_queue);
}

}
}