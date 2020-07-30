#include "unirender/vulkan/Utility.h"

#include <stdexcept>

namespace ur
{
namespace vulkan
{

void Utility::CreateBuffer(VkDevice device, VkPhysicalDevice phy_dev, VkDeviceSize size, VkBufferUsageFlags usage, 
                           VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory)
{
    VkBufferCreateInfo buf_ci{};
    buf_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_ci.size = size;
    buf_ci.usage = usage;
    buf_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &buf_ci, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device, buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = FindMemoryType(phy_dev, mem_reqs.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    if (vkBindBufferMemory(device, buffer, buffer_memory, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to bind buffer memory!");
    }
}

uint32_t Utility::FindMemoryType(VkPhysicalDevice phy_dev, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(phy_dev, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

}
}