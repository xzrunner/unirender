#include "unirender/vulkan/DeviceInfo.h"

#include <iostream>
#include <sstream>

#include <assert.h>

namespace ur
{
namespace vulkan
{

void DeviceInfo::Init(void* hwnd)
{
    InitGlobalLayerProperties();
    InitInstanceExtensionNames();
    InitDeviceExtensionNames();
    InitInstance("unirender-vulkan");
    InitEnumerateDevice();
    InitWindowSize(20, 20);
    InitSwapchainExtension(hwnd);
    InitDevice();
    InitDeviceQueue();
}

VkResult DeviceInfo::InitGlobalLayerProperties()
{
    uint32_t instance_layer_count;
    VkLayerProperties *vk_props = NULL;
    VkResult res;
#ifdef __ANDROID__
    // This place is the first place for samples to use Vulkan APIs.
    // Here, we are going to open Vulkan.so on the device and retrieve function pointers using
    // vulkan_wrapper helper.
    if (!InitVulkan()) {
        LOGE("Failied initializing Vulkan APIs!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    LOGI("Loaded Vulkan APIs.");
#endif

    /*
     * It's possible, though very rare, that the number of
     * instance layers could change. For example, installing something
     * could include new layers that the loader would pick up
     * between the initial query for the count and the
     * request for VkLayerProperties. The loader indicates that
     * by returning a VK_INCOMPLETE status and will update the
     * the count parameter.
     * The count parameter will be updated with the number of
     * entries loaded into the data pointer - in case the number
     * of layers went down or is smaller than the size given.
     */
    do {
        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        if (res) return res;

        if (instance_layer_count == 0) {
            return VK_SUCCESS;
        }

        vk_props = (VkLayerProperties *)realloc(vk_props, instance_layer_count * sizeof(VkLayerProperties));

        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
    } while (res == VK_INCOMPLETE);

    /*
     * Now gather the extension list for each instance layer.
     */
    for (uint32_t i = 0; i < instance_layer_count; i++) {
        layer_properties layer_props;
        layer_props.properties = vk_props[i];
        res = InitGlobalExtensionProperties(layer_props);
        if (res) return res;
        instance_layer_properties.push_back(layer_props);
    }
    free(vk_props);

    return res;
}

void DeviceInfo::InitInstanceExtensionNames()
{
    instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef __ANDROID__
    instance_extension_names.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32)
    instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    instance_extension_names.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    instance_extension_names.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
    instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
}

void DeviceInfo::InitDeviceExtensionNames()
{
    device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VkResult DeviceInfo::InitInstance(const char* title)
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = title;
    app_info.applicationVersion = 1;
    app_info.pEngineName = title;
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_ci = {};
    instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_ci.pNext = NULL;
    instance_ci.flags = 0;
    instance_ci.pApplicationInfo = &app_info;
    instance_ci.enabledLayerCount = instance_layer_names.size();
    instance_ci.ppEnabledLayerNames = instance_layer_names.size() ? instance_layer_names.data() : NULL;
    instance_ci.enabledExtensionCount = instance_extension_names.size();
    instance_ci.ppEnabledExtensionNames = instance_extension_names.data();

    VkResult res = vkCreateInstance(&instance_ci, NULL, &inst);
    assert(res == VK_SUCCESS);

    return res;
}

VkResult DeviceInfo::InitEnumerateDevice(uint32_t gpu_count)
{
    uint32_t const req_count = gpu_count;
    VkResult res = vkEnumeratePhysicalDevices(inst, &gpu_count, NULL);
    assert(gpu_count);
    gpus.resize(gpu_count);

    res = vkEnumeratePhysicalDevices(inst, &gpu_count, gpus.data());
    assert(!res && gpu_count >= req_count);

    vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_family_count, NULL);
    assert(queue_family_count >= 1);

    queue_props.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpus[0], &queue_family_count, queue_props.data());
    assert(queue_family_count >= 1);

    /* This is as good a place as any to do this */
    vkGetPhysicalDeviceMemoryProperties(gpus[0], &memory_properties);
    vkGetPhysicalDeviceProperties(gpus[0], &gpu_props);
    /* query device extensions for enabled layers */
    for (auto &layer_props : instance_layer_properties) {
        InitDeviceExtensionProperties(layer_props);
    }

    return res;
}

void DeviceInfo::InitWindowSize(int width, int height)
{
    this->width = width;
    this->height = height;
}

void DeviceInfo::InitSwapchainExtension(void* hwnd)
{
    /* DEPENDS on init_connection() and init_window() */

    VkResult res;

// Construct the surface description:
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd = (HWND)hwnd;
    res = vkCreateWin32SurfaceKHR(inst, &createInfo, NULL, &surface);
#else
    return;
#endif  // _WIN32
    assert(res == VK_SUCCESS);

    // Iterate over each queue to learn whether it supports presenting:
    VkBool32 *pSupportsPresent = (VkBool32 *)malloc(queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < queue_family_count; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], i, surface, &pSupportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    graphics_queue_family_index = UINT32_MAX;
    present_queue_family_index = UINT32_MAX;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphics_queue_family_index == UINT32_MAX) graphics_queue_family_index = i;

            if (pSupportsPresent[i] == VK_TRUE) {
                graphics_queue_family_index = i;
                present_queue_family_index = i;
                break;
            }
        }
    }

    if (present_queue_family_index == UINT32_MAX) {
        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (size_t i = 0; i < queue_family_count; ++i)
            if (pSupportsPresent[i] == VK_TRUE) {
                present_queue_family_index = i;
                break;
            }
    }
    free(pSupportsPresent);

    // Generate error if could not find queues that support graphics
    // and present
    if (graphics_queue_family_index == UINT32_MAX || present_queue_family_index == UINT32_MAX) {
        std::cout << "Could not find a queues for both graphics and present";
        exit(-1);
    }

    // Get the list of VkFormats that are supported:
    uint32_t formatCount;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, NULL);
    assert(res == VK_SUCCESS);
    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, surfFormats);
    assert(res == VK_SUCCESS);
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
        format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        assert(formatCount >= 1);
        format = surfFormats[0].format;
    }
    free(surfFormats);
}

VkResult DeviceInfo::InitDevice()
{
    VkResult res;
    VkDeviceQueueCreateInfo queue_info = {};

    float queue_priorities[1] = {0.0};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = queue_priorities;
    queue_info.queueFamilyIndex = graphics_queue_family_index;

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = NULL;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = device_extension_names.size();
    device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount ? device_extension_names.data() : NULL;
    device_info.pEnabledFeatures = NULL;

    res = vkCreateDevice(gpus[0], &device_info, NULL, &device);
    assert(res == VK_SUCCESS);

    return res;
}

void DeviceInfo::InitDeviceQueue()
{
    vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue);
    if (graphics_queue_family_index == present_queue_family_index) {
        present_queue = graphics_queue;
    } else {
        vkGetDeviceQueue(device, present_queue_family_index, 0, &present_queue);
    }
}

VkResult DeviceInfo::InitGlobalExtensionProperties(layer_properties& layer_props)
{
    VkExtensionProperties *instance_extensions;
    uint32_t instance_extension_count;
    VkResult res;
    char *layer_name = NULL;

    layer_name = layer_props.properties.layerName;

    do {
        res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, NULL);
        if (res) return res;

        if (instance_extension_count == 0) {
            return VK_SUCCESS;
        }

        layer_props.instance_extensions.resize(instance_extension_count);
        instance_extensions = layer_props.instance_extensions.data();
        res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, instance_extensions);
    } while (res == VK_INCOMPLETE);

    return res;
}

VkResult DeviceInfo::InitDeviceExtensionProperties(layer_properties& layer_props)
{
    VkExtensionProperties *device_extensions;
    uint32_t device_extension_count;
    VkResult res;
    char *layer_name = NULL;

    layer_name = layer_props.properties.layerName;

    do {
        res = vkEnumerateDeviceExtensionProperties(gpus[0], layer_name, &device_extension_count, NULL);
        if (res) return res;

        if (device_extension_count == 0) {
            return VK_SUCCESS;
        }

        layer_props.device_extensions.resize(device_extension_count);
        device_extensions = layer_props.device_extensions.data();
        res = vkEnumerateDeviceExtensionProperties(gpus[0], layer_name, &device_extension_count, device_extensions);
    } while (res == VK_INCOMPLETE);

    return res;
}

}
}