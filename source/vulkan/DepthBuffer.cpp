#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Utility.h"

#include <iostream>

#include <assert.h>

#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

namespace ur
{
namespace vulkan
{

DepthBuffer::DepthBuffer(const std::shared_ptr<LogicalDevice>& device,
                         const PhysicalDevice& phy_dev, int width, int height)
    : m_device(device)
{
    auto format = VK_FORMAT_D16_UNORM;

    VkResult res;
    VkImageCreateInfo image_info = {};
    VkFormatProperties props;

    /* allow custom depth formats */
    m_format = VK_FORMAT_D16_UNORM;

    const VkFormat depth_format = m_format;
    vkGetPhysicalDeviceFormatProperties(phy_dev.GetHandler(), depth_format, &props);
    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        image_info.tiling = VK_IMAGE_TILING_LINEAR;
    } else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    } else {
        /* Try other depth formats? */
        std::cout << "depth_format " << depth_format << " Unsupported.\n";
        exit(-1);
    }

    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = NULL;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = depth_format;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = NUM_SAMPLES;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = NULL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_info.flags = 0;

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.image = VK_NULL_HANDLE;
    view_info.format = depth_format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.flags = 0;

    if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
        depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkMemoryRequirements mem_reqs;

    auto dev = m_device->GetHandler();

    /* Create image */
    res = vkCreateImage(dev, &image_info, NULL, &m_image);
    assert(res == VK_SUCCESS);

    vkGetImageMemoryRequirements(dev, m_image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = Utility::FindMemoryType(
        phy_dev.GetHandler(), mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    /* Allocate memory */
    res = vkAllocateMemory(dev, &mem_alloc, NULL, &m_mem);
    assert(res == VK_SUCCESS);

    /* Bind memory */
    res = vkBindImageMemory(dev, m_image, m_mem, 0);
    assert(res == VK_SUCCESS);

    /* Create image view */
    view_info.image = m_image;
    res = vkCreateImageView(dev, &view_info, NULL, &m_view);
    assert(res == VK_SUCCESS);
}

DepthBuffer::~DepthBuffer()
{
    auto dev = m_device->GetHandler();
    vkDestroyImageView(dev, m_view, NULL);
    vkDestroyImage(dev, m_image, NULL);
    vkFreeMemory(dev, m_mem, NULL);
}

}
}