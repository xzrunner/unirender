#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/Instance.h"
#include "unirender/vulkan/Surface.h"

#include <vector>
#include <set>

namespace
{

const std::vector<const char*> DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

}

namespace ur
{
namespace vulkan
{

PhysicalDevice::PhysicalDevice(const Instance& instance, const Surface* surface)
{
    auto vk_inst = instance.GetHandler();

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vk_inst, &device_count, nullptr);

    if (device_count == 0) {
        throw std::runtime_error("failed to find GPUs with PhysicalDevice support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(vk_inst, &device_count, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device, surface)) {
            m_handle = device;
            break;
        }
    }

    if (m_handle == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

uint32_t PhysicalDevice::FindMemoryType(VkPhysicalDevice phy_dev, uint32_t type_filter,
                                        VkMemoryPropertyFlags properties)
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

const std::vector<const char*>& 
PhysicalDevice::GetDeviceExtensions()
{
    return DeviceExtensions;
}

PhysicalDevice::QueueFamilyIndices 
PhysicalDevice::FindQueueFamilies(VkPhysicalDevice device, const Surface* surface)
{
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto& queue_family : queue_families) 
    {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        if (surface)
        {
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface->GetHandler(), &present_support);

            if (present_support) {
                indices.present_family = i;
            }
        }

        if (indices.IsComplete(surface)) {
            break;
        }

        i++;
    }

    return indices;
}

bool PhysicalDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool PhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device, const Surface* surface)
{
    QueueFamilyIndices indices = FindQueueFamilies(device, surface);

    bool extensions_supported = CheckDeviceExtensionSupport(device);

    if (surface)
    {
        bool swap_chain_adequate = false;
        if (extensions_supported) {
            Swapchain::SwapChainSupportDetails swap_chain_support
                = Swapchain::QuerySwapChainSupport(device, surface->GetHandler());
            swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
        }

        return indices.IsComplete(surface) && extensions_supported && swap_chain_adequate;
    }
    else
    {
        return indices.IsComplete(surface) && extensions_supported;
    }
}

}
}