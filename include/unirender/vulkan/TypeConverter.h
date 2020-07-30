#pragma once

#include "unirender/ShaderType.h"
#include "unirender/DescriptorType.h"
#include "unirender/TextureTarget.h"
#include "unirender/TextureFormat.h"
#include "unirender/TextureWrap.h"
#include "unirender/TextureMinificationFilter.h"
#include "unirender/TextureMagnificationFilter.h"

#include <vulkan/vulkan.h>

#include <assert.h>

namespace ur
{
namespace vulkan
{

class TypeConverter
{
public:
    static VkShaderStageFlagBits To(ShaderType type)
    {
        const VkShaderStageFlagBits types[] = {
            VK_SHADER_STAGE_VERTEX_BIT,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_SHADER_STAGE_COMPUTE_BIT,
        };
        return types[static_cast<int>(type)];
    }

    static VkDescriptorType To(DescriptorType type)
    {
        const VkDescriptorType types[] = {
            VK_DESCRIPTOR_TYPE_SAMPLER,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
            VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        };
        return types[static_cast<int>(type)];
    }

    static VkImageType To(TextureTarget tex_target)
    {
        assert(tex_target >= TextureTarget::Texture1D 
            && tex_target <= TextureTarget::Texture3D);
        const VkImageType types[] = {
            VK_IMAGE_TYPE_1D,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_TYPE_3D
        };
        return types[static_cast<int>(tex_target)];
    }

    static VkFormat To(TextureFormat fmt)
    {
        const VkFormat fmts[] = {
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_FORMAT_R4G4B4A4_UNORM_PACK16,
            VK_FORMAT_R8G8B8_UNORM,
            VK_FORMAT_R5G6B5_UNORM_PACK16,
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_B8G8R8_UNORM,
            VK_FORMAT_R16G16B16A16_USCALED,
            VK_FORMAT_R16G16B16_USCALED,
            VK_FORMAT_R32G32B32_SFLOAT,
            VK_FORMAT_R16G16_SFLOAT,
            VK_FORMAT_R8_UNORM,          // A8
            VK_FORMAT_R8_UNORM,
            VK_FORMAT_R16_UINT,
            VK_FORMAT_D16_UNORM,
            VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
            VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
            VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,    // ETC1
            VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
            // COMPRESSED_RGBA_S3TC_DXT1_EXT,
            // COMPRESSED_RGBA_S3TC_DXT3_EXT,
            // COMPRESSED_RGBA_S3TC_DXT5_EXT,
        };
        return fmts[static_cast<int>(fmt)];
    }

    static VkSamplerAddressMode To(TextureWrap wrap)
    {
        const VkSamplerAddressMode wraps[] = {
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            VK_SAMPLER_ADDRESS_MODE_REPEAT,
            VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        };
        return wraps[static_cast<int>(wrap)];
    }

    static VkFilter To(TextureMinificationFilter filter)
    {
        assert(filter == TextureMinificationFilter::Nearest 
            || filter == TextureMinificationFilter::Linear);
        const VkFilter filters[] = {
            VK_FILTER_NEAREST,
            VK_FILTER_LINEAR,
        };
        return filters[static_cast<int>(filter)];
    }

    static VkFilter To(TextureMagnificationFilter filter)
    {
        assert(filter == TextureMagnificationFilter::Nearest
            || filter == TextureMagnificationFilter::Linear);
        const VkFilter filters[] = {
            VK_FILTER_NEAREST,
            VK_FILTER_LINEAR,
        };
        return filters[static_cast<int>(filter)];
    }

}; // TypeConverter

}
}