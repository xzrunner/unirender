#pragma once

#include "unirender/Texture.h"
#include "unirender/TextureDescription.h"
#include "unirender/TextureTarget.h"

#include <vulkan/vulkan.h>

namespace ur
{

class Device;

namespace vulkan
{

class Image;
class ImageView;
class LogicalDevice;
class PhysicalDevice;
class CommandPool;

class Texture : public ur::Texture
{
public:
    Texture(const std::shared_ptr<LogicalDevice>& device, 
        const std::shared_ptr<PhysicalDevice>& phy_dev,
        const std::shared_ptr<ur::TextureSampler>& sampler);
    virtual ~Texture();

    virtual int GetTexID() const override { return 0; }

    virtual int GetWidth() const override { return m_desc.width; }
    virtual int GetHeight() const override { return m_desc.height; }
    virtual int GetDepth() const override { return m_desc.depth; }

    virtual TextureFormat GetFormat() const override { return m_desc.format; }

    virtual void Bind() const override;
    static void UnBind(TextureTarget target);

    virtual void Upload(const void* pixels, int x, int y, int w, int h,
        int miplevel = 0, int row_alignment = 4) override;

    virtual void ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler) override;

    void ReadFromMemory(const TextureDescription& desc, const std::shared_ptr<CommandPool>& cmd_pool,
        const void* pixels, int row_alignment, int mip_level = 0);

    auto& GetDescInfo() const { return m_vk_desc; }

private:
    std::shared_ptr<LogicalDevice>  m_device;
    std::shared_ptr<PhysicalDevice> m_phy_dev;

	TextureDescription m_desc;

    std::shared_ptr<Image>     m_image      = nullptr;
    std::shared_ptr<ImageView> m_image_view = nullptr;

    std::shared_ptr<ur::TextureSampler> m_sampler = nullptr;

    VkDescriptorImageInfo m_vk_desc;

}; // Texture

}
}