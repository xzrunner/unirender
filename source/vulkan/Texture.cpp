#define NOMINMAX

#include "unirender/vulkan/Texture.h"
#include "unirender/vulkan/Image.h"
#include "unirender/vulkan/ImageView.h"
#include "unirender/vulkan/TypeConverter.h"
#include "unirender/vulkan/TextureSampler.h"
#include "unirender/TextureUtility.h"

#include <algorithm>

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

}

void Texture::ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler)
{

}

void Texture::ReadFromMemory(const TextureDescription& desc, const std::shared_ptr<CommandPool>& cmd_pool,
	                         const void* pixels, int row_alignment, int mip_level)
{
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

}
}