#pragma once

#include "unirender/ShaderType.h"
#include "unirender/DescriptorType.h"

#include <vulkan/vulkan.h>

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

}; // TypeConverter

}
}