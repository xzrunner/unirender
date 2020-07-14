#include "unirender/vulkan/DeviceInfo.h"

#include <iostream>
#include <sstream>

#include <assert.h>

namespace debug
{
PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
VkDebugUtilsMessengerEXT debugUtilsMessenger;

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	// Select prefix depending on flags passed to the callback
	std::string prefix("");

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		prefix = "VERBOSE: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		prefix = "INFO: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		prefix = "WARNING: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		prefix = "ERROR: ";
	}


	// Display message to default output (console/logcat)
	std::stringstream debugMessage;
	debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "][" << pCallbackData->pMessageIdName << "] : " << pCallbackData->pMessage;

#if defined(__ANDROID__)
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		LOGE("%s", debugMessage.str().c_str());
	} else {
		LOGD("%s", debugMessage.str().c_str());
	}
#else
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		std::cerr << debugMessage.str() << "\n";
	} else {
		std::cout << debugMessage.str() << "\n";
	}
	fflush(stdout);
#endif


	// The return value of this callback controls wether the Vulkan call that caused the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}

void setupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack)
{
    vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

	VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
	debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugUtilsMessengerCI.pfnUserCallback = debugUtilsMessengerCallback;
	VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCI, nullptr, &debugUtilsMessenger);
	assert(result == VK_SUCCESS);
}

void freeDebugCallback(VkInstance instance)
{
	if (debugUtilsMessenger != VK_NULL_HANDLE)
	{
		vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
	}
}
}

namespace ur
{
namespace vulkan
{

DeviceInfo::~DeviceInfo()
{
    vkDestroyInstance(inst, NULL);

    if (m_validation)
    {
        debug::freeDebugCallback(inst);
    }
}

void DeviceInfo::Init(bool validation)
{
    InitGlobalLayerProperties();
    InitInstanceExtensionNames(validation);
    InitDeviceExtensionNames();
    InitInstance("unirender-vulkan", validation);
    InitEnumerateDevice();

    // If requested, we enable the default validation layers for debugging
    m_validation = validation;
    if (validation)
    {
        // The report flags determine what type of messages for the layers will be displayed
        // For validating (debugging) an appplication the error and warning bits should suffice
        VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        // Additional flags include performance info, loader and layer debug messages, etc.
        debug::setupDebugging(inst, debugReportFlags, VK_NULL_HANDLE);
    }
}

void DeviceInfo::InitDeviceAndQueue(uint32_t graphics_queue_family_index, 
                                    uint32_t present_queue_family_index)
{
    if (device != VK_NULL_HANDLE) {
        return;
    }

    InitDevice(graphics_queue_family_index);
    InitDeviceQueue(graphics_queue_family_index, present_queue_family_index);
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

void DeviceInfo::InitInstanceExtensionNames(bool validation)
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

	if (instance_extension_names.size() > 0 && validation) {
        instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	} 
}

void DeviceInfo::InitDeviceExtensionNames()
{
    device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VkResult DeviceInfo::InitInstance(const char* title, bool validation)
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
    //instance_ci.enabledLayerCount = instance_layer_names.size();
    //instance_ci.ppEnabledLayerNames = instance_layer_names.size() ? instance_layer_names.data() : NULL;
    instance_ci.enabledExtensionCount = instance_extension_names.size();
    instance_ci.ppEnabledExtensionNames = instance_extension_names.data();

	if (validation)
	{
		// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
		// Note that on Android this layer requires at least NDK r20
        const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
		//const char* validationLayerName = "VK_LAYER_LUNARG_standard_validation";
		// Check if this layer is available at instance level
		uint32_t instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
		bool validationLayerPresent = false;
		for (VkLayerProperties layer : instanceLayerProperties) {
			if (strcmp(layer.layerName, validationLayerName) == 0) {
				validationLayerPresent = true;
				break;
			}
		}
		if (validationLayerPresent) {
            instance_ci.ppEnabledLayerNames = &validationLayerName;
            instance_ci.enabledLayerCount = 1;
		} else {
			std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
		}
	}

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

VkResult DeviceInfo::InitDevice(uint32_t graphics_queue_family_index)
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

void DeviceInfo::InitDeviceQueue(uint32_t graphics_queue_family_index,
                                 uint32_t present_queue_family_index)
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