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
        // MUST match ur::ShaderType order exactly:
        // { VertexShader, TessCtrlShader, TessEvalShader, GeometryShader,
        //   FragmentShader, ComputeShader }. The old table had 4 wrongly-ordered
        // entries, so FragmentShader (index 4) read out of bounds and produced a
        // garbage stage flag -> the fragment stage was silently dropped from every
        // pipeline -> rasterization wrote depth but never any color (gray screen,
        // black atlases, "texture bound but never accessed").
        const VkShaderStageFlagBits types[] = {
            VK_SHADER_STAGE_VERTEX_BIT,                  // VertexShader
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,    // TessCtrlShader
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, // TessEvalShader
            VK_SHADER_STAGE_GEOMETRY_BIT,                // GeometryShader
            VK_SHADER_STAGE_FRAGMENT_BIT,                // FragmentShader
            VK_SHADER_STAGE_COMPUTE_BIT,                 // ComputeShader
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
        // MUST match ur::TextureFormat exactly. The old array-indexed-by-enum was short
        // and mis-ordered, so everything past RGBA8 mapped to the wrong VkFormat (e.g.
        // RGBA16F -> R16G16B16_USCALED, R16F -> out of bounds). That gave the deferred
        // GBuffer's gDepth(r16f)/gNormal(rgb16f) garbage/unsupported formats -> no GPU
        // image -> null imageView -> the post-process passes sampled invalid textures.
        // 3-component formats (RGB*, RGB16F...) are promoted to 4 components: Vulkan /
        // MoltenVK rarely support 3-component sampled/renderable images (the Metal and
        // GL backends promote them the same way).
        switch (fmt)
        {
        case TextureFormat::RGBA8:    return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA4:    return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case TextureFormat::RGB:      return VK_FORMAT_R8G8B8A8_UNORM;      // promote 3->4
        case TextureFormat::RGB565:   return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case TextureFormat::BGRA_EXT: return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::BGR_EXT:  return VK_FORMAT_B8G8R8A8_UNORM;      // promote 3->4
        case TextureFormat::RGBA16:   return VK_FORMAT_R16G16B16A16_UNORM;
        case TextureFormat::RGBA16F:  return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RGBA32F:  return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::RGB16F:   return VK_FORMAT_R16G16B16A16_SFLOAT; // promote 3->4
        case TextureFormat::RGB32F:   return VK_FORMAT_R32G32B32A32_SFLOAT; // promote 3->4
        case TextureFormat::RG16F:    return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RG32F:    return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::A8:       return VK_FORMAT_R8_UNORM;
        case TextureFormat::RED:      return VK_FORMAT_R8_UNORM;
        case TextureFormat::R16:      return VK_FORMAT_R16_UNORM;
        case TextureFormat::R16F:     return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::R32F:     return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::DEPTH:    return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::RGB32I:   return VK_FORMAT_R32G32B32A32_SINT;   // promote 3->4
        case TextureFormat::PVR2:     return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
        case TextureFormat::PVR4:     return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
        case TextureFormat::ETC1:     return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case TextureFormat::ETC2:     return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        default:                      return VK_FORMAT_R8G8B8A8_UNORM;
        }
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
        // GL-style min filters fold the in-level filter and the between-mip filter
        // into one enum; Vulkan splits them (VkFilter minFilter + VkSamplerMipmapMode).
        // Here we return just the in-level (NEAREST/LINEAR) component.
        switch (filter)
        {
        case TextureMinificationFilter::Nearest:
        case TextureMinificationFilter::NearestMipmapNearest:
        case TextureMinificationFilter::NearestMipmapLinear:
            return VK_FILTER_NEAREST;
        case TextureMinificationFilter::Linear:
        case TextureMinificationFilter::LinearMipmapNearest:
        case TextureMinificationFilter::LinearMipmapLinear:
        default:
            return VK_FILTER_LINEAR;
        }
    }

    static VkSamplerMipmapMode ToMipmapMode(TextureMinificationFilter filter)
    {
        switch (filter)
        {
        case TextureMinificationFilter::NearestMipmapLinear:
        case TextureMinificationFilter::LinearMipmapLinear:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
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