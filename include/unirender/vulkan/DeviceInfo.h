#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class DeviceInfo
{
public:
    ~DeviceInfo();

    void Init(bool validation = false);
    void InitDeviceAndQueue(uint32_t graphics_queue_family_index, 
        uint32_t present_queue_family_index);

public:
    /*
     * A layer can expose extensions, keep track of those
     * extensions here.
     */
    typedef struct {
        VkLayerProperties properties;
        std::vector<VkExtensionProperties> instance_extensions;
        std::vector<VkExtensionProperties> device_extensions;
    } layer_properties;

public:
    VkResult InitGlobalLayerProperties();
    void InitInstanceExtensionNames(bool validation = false);
    void InitDeviceExtensionNames();
    VkResult InitInstance(const char* title, bool validation = false);
    VkResult InitEnumerateDevice(uint32_t gpu_count = 1);
    void InitWindowSize(int width, int height);
    VkResult InitDevice(uint32_t graphics_queue_family_index);
    void InitDeviceQueue(uint32_t graphics_queue_family_index,
        uint32_t present_queue_family_index);

    VkResult InitGlobalExtensionProperties(layer_properties& layer_props);
    VkResult InitDeviceExtensionProperties(layer_properties& layer_props);

public:
    std::vector<const char*> instance_layer_names;
    std::vector<const char*> instance_extension_names;
    std::vector<layer_properties> instance_layer_properties;
    std::vector<VkExtensionProperties> instance_extension_properties;
    VkInstance inst;

    std::vector<const char*> device_extension_names;
    std::vector<VkExtensionProperties> device_extension_properties;
    std::vector<VkPhysicalDevice> gpus;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties gpu_props;
    std::vector<VkQueueFamilyProperties> queue_props;
    VkPhysicalDeviceMemoryProperties memory_properties;

    uint32_t queue_family_count = 0;

	struct {
		VkDescriptorImageInfo image_info;
	} texture_data;

    VkQueue graphics_queue = VK_NULL_HANDLE;
    VkQueue present_queue = VK_NULL_HANDLE;

private:
    bool m_validation = false;

}; // DeviceInfo

}
}