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
    void Init(void* hwnd);

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
    void InitInstanceExtensionNames();
    void InitDeviceExtensionNames();
    VkResult InitInstance(const char* title);
    VkResult InitEnumerateDevice(uint32_t gpu_count = 1);
    void InitWindowSize(int width, int height);
    void InitSwapchainExtension(void* hwnd);
    VkResult InitDevice();
    void InitDeviceQueue();

    VkResult InitGlobalExtensionProperties(layer_properties& layer_props);
    VkResult InitDeviceExtensionProperties(layer_properties& layer_props);

public:
    VkSurfaceKHR surface;

    std::vector<const char*> instance_layer_names;
    std::vector<const char*> instance_extension_names;
    std::vector<layer_properties> instance_layer_properties;
    std::vector<VkExtensionProperties> instance_extension_properties;
    VkInstance inst;

    std::vector<const char*> device_extension_names;
    std::vector<VkExtensionProperties> device_extension_properties;
    std::vector<VkPhysicalDevice> gpus;
    VkDevice device;
    uint32_t graphics_queue_family_index;
    uint32_t present_queue_family_index;
    VkPhysicalDeviceProperties gpu_props;
    std::vector<VkQueueFamilyProperties> queue_props;
    VkPhysicalDeviceMemoryProperties memory_properties;

    int width, height;
    VkFormat format;

	uint32_t current_buffer     = 0;
    uint32_t queue_family_count = 0;

	struct {
		VkDescriptorImageInfo image_info;
	} texture_data;

    VkQueue graphics_queue;
    VkQueue present_queue;

}; // DeviceInfo

}
}