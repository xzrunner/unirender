#pragma once

#include "unirender/ShaderType.h"

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

}; // TypeConverter

}
}