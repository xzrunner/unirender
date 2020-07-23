#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/Swapchain.h"

#include <vector>
#include <set>

namespace
{

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

}

namespace ur
{
namespace vulkan
{

PhysicalDevice::PhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
	: m_instance(instance)
    , m_surface(surface)
{
}

PhysicalDevice::~PhysicalDevice()
{
}

void PhysicalDevice::Create()
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

    if (device_count == 0) {
        throw std::runtime_error("failed to find GPUs with PhysicalDevice support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
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

PhysicalDevice::QueueFamilyIndices 
PhysicalDevice::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

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

bool PhysicalDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool PhysicalDevice::IsDeviceSuitable(VkPhysicalDevice device) const
{
    QueueFamilyIndices indices = FindQueueFamilies(device, m_surface);

    bool extensions_supported = CheckDeviceExtensionSupport(device);

    if (m_surface)
    {
        bool swap_chain_adequate = false;
        if (extensions_supported) {
            Swapchain::SwapChainSupportDetails swap_chain_support
                = Swapchain::QuerySwapChainSupport(device, m_surface);
            swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
        }

        return indices.IsComplete(m_surface) && extensions_supported && swap_chain_adequate;
    }
    else
    {
        return indices.IsComplete(m_surface) && extensions_supported;
    }
}

}
}