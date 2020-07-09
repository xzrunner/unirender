#include "unirender/vulkan/Utility.h"
#include "unirender/vulkan/DeviceInfo.h"

namespace ur
{
namespace vulkan
{

bool Utility::MemoryTypeFromProperties(const DeviceInfo& dev_info, uint32_t typeBits,
                                       VkFlags requirements_mask, uint32_t *typeIndex)
{
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < dev_info.memory_properties.memoryTypeCount; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((dev_info.memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

}
}