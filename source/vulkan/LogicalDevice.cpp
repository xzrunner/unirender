#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/ValidationLayers.h"
#include "unirender/vulkan/Surface.h"

#include <vector>
#include <set>
#include <stdexcept>

namespace ur
{
namespace vulkan
{

LogicalDevice::LogicalDevice(bool enable_validation_layers, const PhysicalDevice& phy_dev, const Surface* surface)
{
    PhysicalDevice::QueueFamilyIndices indices = PhysicalDevice::FindQueueFamilies(phy_dev.GetHandler(), surface);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

    std::set<uint32_t> unique_queue_families = { indices.graphics_family.value() };
    if (surface) {
        unique_queue_families.insert(indices.present_family.value());
    }

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_ci{};
        queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_ci.queueFamilyIndex = queueFamily;
        queue_ci.queueCount = 1;
        queue_ci.pQueuePriorities = &queuePriority;
        queue_create_infos.push_back(queue_ci);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo device_ci{};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    device_ci.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_ci.pQueueCreateInfos = queue_create_infos.data();

    device_ci.pEnabledFeatures = &deviceFeatures;

    auto& dev_exts = PhysicalDevice::GetDeviceExtensions();
    device_ci.enabledExtensionCount = dev_exts.size();
    device_ci.ppEnabledExtensionNames = dev_exts.data();

    if (enable_validation_layers)
    {
        auto& validation_layers = ValidationLayers::GetValidationLayers();
        device_ci.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        device_ci.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        device_ci.enabledLayerCount = 0;
    }

    if (vkCreateDevice(phy_dev.GetHandler(), &device_ci, nullptr, &m_handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_handle, indices.graphics_family.value(), 0, &m_graphics_queue);
    if (surface) {
        vkGetDeviceQueue(m_handle, indices.present_family.value(), 0, &m_present_queue);
    }
}

LogicalDevice::~LogicalDevice()
{
	vkDestroyDevice(m_handle, nullptr);
}

}
}