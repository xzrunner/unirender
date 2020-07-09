#pragma once

#include <vulkan/vulkan.h>

/* Number of descriptor sets needs to be the same at alloc,       */
/* pipeline layout creation, and descriptor set layout creation   */
#define NUM_DESCRIPTOR_SETS 1

/* Number of samples needs to be the same at image creation,      */
/* renderpass creation and pipeline creation.                     */
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

/* Number of viewports and number of scissors have to be the same */
/* at pipeline creation and in any call to set them dynamically   */
/* They also have to be the same as each other                    */
#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

namespace ur
{
namespace vulkan
{

class DeviceInfo;

class Utility
{
public:
	static bool MemoryTypeFromProperties(const DeviceInfo& dev_info, uint32_t typeBits,
		VkFlags requirements_mask, uint32_t* typeIndex);

}; // Utility

}
}