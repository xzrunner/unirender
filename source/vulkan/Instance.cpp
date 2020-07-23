#include "unirender/vulkan/Instance.h"
#include "unirender/vulkan/ValidationLayers.h"

namespace
{

std::vector<const char*> GetRequiredExtensions(bool enable_validation_layers)
{
    std::vector<const char*> instance_extension_names;
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

    if (instance_extension_names.size() > 0 && enable_validation_layers) {
        instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return instance_extension_names;
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

    if (vkCreateInstance(&createInfo, nullptr, &m_handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create m_handle!");
    }
}

Instance::~Instance()
{
	vkDestroyInstance(m_handle, nullptr);
}

}
}