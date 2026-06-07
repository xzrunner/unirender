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
	// Free any buffers still pending deferred destroy before tearing down the device.
	// The owning Context already waited the GPU idle in its destructor.
	CollectAllRetired();
	vkDestroyDevice(m_handle, nullptr);
}

void LogicalDevice::RetireBuffer(VkBuffer buffer, VkDeviceMemory memory)
{
	if (buffer == VK_NULL_HANDLE && memory == VK_NULL_HANDLE) {
		return;
	}
	m_retired.push_back({ buffer, memory, m_cur_frame });
}

void LogicalDevice::CollectRetired(uint64_t completed_frame)
{
	for (size_t i = 0; i < m_retired.size(); )
	{
		if (m_retired[i].frame <= completed_frame)
		{
			if (m_retired[i].buffer != VK_NULL_HANDLE) {
				vkDestroyBuffer(m_handle, m_retired[i].buffer, nullptr);
			}
			if (m_retired[i].memory != VK_NULL_HANDLE) {
				vkFreeMemory(m_handle, m_retired[i].memory, nullptr);
			}
			// Order is irrelevant for destruction -- swap-erase with the tail.
			m_retired[i] = m_retired.back();
			m_retired.pop_back();
		}
		else
		{
			++i;
		}
	}
}

void LogicalDevice::CollectAllRetired()
{
	if (m_handle == VK_NULL_HANDLE) {
		return;
	}
	for (auto& r : m_retired)
	{
		if (r.buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(m_handle, r.buffer, nullptr);
		}
		if (r.memory != VK_NULL_HANDLE) {
			vkFreeMemory(m_handle, r.memory, nullptr);
		}
	}
	m_retired.clear();
}

}
}