#define NOMINMAX

#include "unirender/vulkan/Texture.h"
#include "unirender/vulkan/Image.h"
#include "unirender/vulkan/ImageView.h"
#include "unirender/vulkan/TypeConverter.h"
#include "unirender/vulkan/TextureSampler.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/CommandBuffer.h"
#include "unirender/TextureUtility.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace ur
{
namespace vulkan
{

Texture::Texture(const std::shared_ptr<LogicalDevice>& device, 
	             const std::shared_ptr<PhysicalDevice>& phy_dev,
	             const std::shared_ptr<ur::TextureSampler>& sampler)
	: m_device(device)
	, m_phy_dev(phy_dev)
	, m_sampler(sampler)
{
}

Texture::~Texture()
{

}

void Texture::Bind() const
{

}

void Texture::UnBind(TextureTarget target)
{

}

void Texture::Upload(const void* pixels, int x, int y, int w, int h, int miplevel, int row_alignment)
{
	// Sub-region CPU->GPU upload. dtex packs glyph/symbol bitmaps into the GUI atlas
	// through this path (PixBufPage::UploadTexture); leaving it a no-op left the atlas
	// black, so the GUI sampled an empty texture and rendered nothing.
	//
	// pixels == nullptr is used by PixBufPage::Clear to wipe the page; we skip it -- the
	// next UploadTexture re-uploads the whole (zeroed) CPU page, which overwrites it.
	if (!m_image || !pixels || w <= 0 || h <= 0) {
		if (std::getenv("TT_VK_PROBE")) {
			fprintf(stderr, "[upload] SKIP image=%p pixels=%p %dx%d\n",
				(void*)m_image.get(), pixels, w, h);
		}
		return;
	}

	const auto size = TextureUtility::RequiredSizeInBytes(w, h, m_desc.format, row_alignment);

	if (std::getenv("TT_VK_PROBE")) {
		const uint8_t* p = static_cast<const uint8_t*>(pixels);
		size_t nz = 0; for (size_t i = 0; i < size; ++i) { if (p[i]) ++nz; }
		fprintf(stderr, "[upload] %dx%d at (%d,%d) size=%zu nonzero=%zu fmt=%d\n",
			w, h, x, y, size, nz, (int)m_desc.format);
	}
	m_image->UploadRegion(*m_phy_dev, pixels, size,
		static_cast<uint32_t>(x), static_cast<uint32_t>(y),
		static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

void Texture::ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler)
{

}

void Texture::ReadFromMemory(const TextureDescription& desc, const std::shared_ptr<CommandPool>& cmd_pool,
	                         const void* pixels, int row_alignment, int mip_level)
{
	m_desc = desc; // FIX: was never assigned -> GetWidth/Height/Format returned defaults

	VkExtent3D sz;
	sz.width  = static_cast<uint32_t>(desc.width);
	sz.height = static_cast<uint32_t>(desc.height);
	sz.depth  = static_cast<uint32_t>(std::max(1, desc.depth));

	auto vk_fmt = TypeConverter::To(desc.format);

	m_image = std::make_shared<Image>(m_device, *m_phy_dev, cmd_pool, TypeConverter::To(desc.target), vk_fmt, sz);

	const auto size = TextureUtility::RequiredSizeInBytes(desc.width, desc.height, m_desc.format, row_alignment);
	m_image->Upload(*m_phy_dev, pixels, size);

	VkImageViewType view_type;
	switch (desc.target)
	{
	case TextureTarget::Texture1D:
		view_type = VK_IMAGE_VIEW_TYPE_1D;
		break;
	case TextureTarget::Texture2D:
		view_type = VK_IMAGE_VIEW_TYPE_2D;
		break;
	case TextureTarget::Texture3D:
		view_type = VK_IMAGE_VIEW_TYPE_3D;
		break;
	default:
		assert(0);
	}

	VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	m_image_view = std::make_shared<ImageView>(m_device, view_type, vk_fmt, range, *m_image);

	m_vk_desc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_vk_desc.imageView = m_image_view->GetHandler();
	m_vk_desc.sampler = std::static_pointer_cast<vulkan::TextureSampler>(m_sampler)->GetHandler();

}

void Texture::CreateAsColorAttachment(const TextureDescription& desc, const std::shared_ptr<CommandPool>& cmd_pool)
{
	m_desc = desc;

	VkExtent3D sz;
	sz.width  = static_cast<uint32_t>(desc.width);
	sz.height = static_cast<uint32_t>(desc.height);
	sz.depth  = 1;

	auto vk_fmt = TypeConverter::To(desc.format);

	m_image = std::make_shared<Image>(m_device, *m_phy_dev, cmd_pool,
		VK_IMAGE_TYPE_2D, vk_fmt, sz, 1, 1, /*as_color_attachment=*/true);

	// One-time: UNDEFINED -> TRANSFER_DST -> clear(0) -> SHADER_READ_ONLY_OPTIMAL,
	// so the first offscreen pass (loadOp = LOAD) sees a defined, zeroed image and
	// the texture is immediately sampleable.
	{
		auto vk_dev = m_device->GetHandler();
		VkCommandBuffer cb = CommandBuffer::BeginSingleTimeCommands(vk_dev, cmd_pool->GetHandler());

		VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkImageMemoryBarrier to_dst = {};
		to_dst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		to_dst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		to_dst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		to_dst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		to_dst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		to_dst.image = m_image->GetHandler();
		to_dst.subresourceRange = range;
		to_dst.srcAccessMask = 0;
		to_dst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &to_dst);

		VkClearColorValue clear = {};
		vkCmdClearColorImage(cb, m_image->GetHandler(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);

		VkImageMemoryBarrier to_read = to_dst;
		to_read.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		to_read.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		to_read.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		to_read.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &to_read);

		CommandBuffer::EndSingleTimeCommands(cb, vk_dev, cmd_pool->GetHandler(), m_device->GetGraphicsQueue());
	}

	VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	m_image_view = std::make_shared<ImageView>(m_device, VK_IMAGE_VIEW_TYPE_2D, vk_fmt, range, *m_image);

	m_vk_desc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_vk_desc.imageView   = m_image_view->GetHandler();
	m_vk_desc.sampler     = std::static_pointer_cast<vulkan::TextureSampler>(m_sampler)->GetHandler();
}

VkImageView Texture::GetImageView() const
{
	return m_image_view ? m_image_view->GetHandler() : VK_NULL_HANDLE;
}

VkFormat Texture::GetVkFormat() const
{
	return TypeConverter::To(m_desc.format);
}

VkImage Texture::GetVkImage() const
{
	return m_image ? m_image->GetHandler() : VK_NULL_HANDLE;
}

}
}