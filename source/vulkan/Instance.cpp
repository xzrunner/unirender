#include "unirender/vulkan/Instance.h"
#include "unirender/vulkan/ValidationLayers.h"

#include <stdexcept>
#include <string>

// The bundled (older) Vulkan headers predate VK_KHR_portability_enumeration, which
// MoltenVK needs to be enumerable by the loader on macOS. Define the name/flag we
// use here as a fallback so it builds against those headers too.
#ifndef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#define VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME "VK_KHR_portability_enumeration"
#endif
#ifndef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#define VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR 0x00000001
#endif

namespace
{

std::vector<const char*> GetRequiredExtensions(bool enable_validation_layers)
{
    std::vector<const char*> instance_extension_names;
    instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    //// Extension VK_KHR_maintenance1 not found in list of known instance extensions.
    //instance_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
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

#ifdef __APPLE__
    // MoltenVK is a "portability" driver: the loader hides it unless we ask for
    // portability enumeration. get_physical_device_properties2 is a dependency of
    // the VK_KHR_portability_subset device extension we enable later.
    instance_extension_names.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    if (instance_extension_names.size() > 0 && enable_validation_layers) {
        instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return instance_extension_names;
}

const char* VkResultToString(VkResult err)
{
    switch (err)
    {
    case VK_SUCCESS:                     return "VK_SUCCESS";
    case VK_NOT_READY:                   return "VK_NOT_READY";
    case VK_TIMEOUT:                     return "VK_TIMEOUT";
    case VK_INCOMPLETE:                  return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:    return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:  return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:     return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:   return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:   return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:      return "VK_ERROR_TOO_MANY_OBJECTS";
    default:                             return "VK_ERROR_<unknown>";
    }
}

}

namespace ur
{
namespace vulkan
{

Instance::Instance(bool enable_validation_layers)
{
   if (enable_validation_layers && !ValidationLayers::CheckValidationLayerSuppor()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "unirender";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
#ifdef __APPLE__
    // Required so the loader will enumerate the MoltenVK (portability) physical device.
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    auto extensions = GetRequiredExtensions(enable_validation_layers);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enable_validation_layers)
    {
        auto& layers_name = ValidationLayers::GetValidationLayers();
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers_name.size());
        createInfo.ppEnabledLayerNames = layers_name.data();

        ValidationLayers::PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    VkResult err = vkCreateInstance(&createInfo, nullptr, &m_handle);
    if (err != VK_SUCCESS) {
        throw std::runtime_error(
            std::string("failed to create Vulkan instance: ") + VkResultToString(err)
            + " (" + std::to_string(static_cast<int>(err)) + ")");
    }
}

Instance::~Instance()
{
	vkDestroyInstance(m_handle, nullptr);
}

}
}