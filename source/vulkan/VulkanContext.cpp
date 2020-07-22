#include "unirender/vulkan/VulkanContext.h"
#include "unirender/vulkan/VulkanDevice.h"
#include "unirender/vulkan/Swapchain.h"

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

VulkanContext::VulkanContext(const VulkanDevice& vk)
    : m_vk_ctx(vk)
{
}

VulkanContext::~VulkanContext()
{
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroyDevice(m_device, nullptr);

    vkDestroySurfaceKHR(m_vk_ctx.GetInstance(), m_surface, nullptr);
}

void VulkanContext::Init(uint32_t width, uint32_t height, void* hwnd)
{
    CreateSurface(hwnd);
    PickPhysicalDevice();
    CreateLogicalDevice();
    //CreateSwapChain(width, height);
}

void VulkanContext::Resize(uint32_t width, uint32_t height)
{
    //CreateSwapChain(width, height);
}

uint32_t VulkanContext::FindMemoryType(VkPhysicalDevice phy_dev, uint32_t type_filter, 
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

void VulkanContext::CreateSurface(void* hwnd) 
{
    VkWin32SurfaceCreateInfoKHR surface_ci = {};
    surface_ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_ci.pNext = NULL;
    surface_ci.hinstance = GetModuleHandle(nullptr);
    surface_ci.hwnd = (HWND)hwnd;
    if (vkCreateWin32SurfaceKHR(m_vk_ctx.GetInstance(), &surface_ci, NULL, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create m_surface!");
    }
}

void VulkanContext::PickPhysicalDevice() 
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vk_ctx.GetInstance(), &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with VulkanContext support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vk_ctx.GetInstance(), &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            m_physical_device = device;
            break;
        }
    }

    if (m_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void VulkanContext::CreateLogicalDevice() 
{
    QueueFamilyIndices indices = FindQueueFamilies(m_physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};

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

    device_ci.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    device_ci.ppEnabledExtensionNames = deviceExtensions.data();

    if (m_vk_ctx.IsEnableValidationLayers()) 
    {
        auto& validation_layers = m_vk_ctx.GetValidationLayers();
        device_ci.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        device_ci.ppEnabledLayerNames = validation_layers.data();
    } 
    else 
    {
        device_ci.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physical_device, &device_ci, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);
}

void VulkanContext::CreateSwapChain(uint32_t width, uint32_t height)
{
    Swapchain::SwapChainSupportDetails swapChainSupport 
        = Swapchain::QuerySwapChainSupport(m_physical_device, m_surface);

    VkSurfaceFormatKHR surfaceFormat = Swapchain::ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = Swapchain::ChooseSwapPresentMode(swapChainSupport.present_modes);
    VkExtent2D extent = Swapchain::ChooseSwapExtent(swapChainSupport.capabilities, width, height);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physical_device);
    uint32_t queueFamilyIndices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchain_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchain_images.data());

    m_swapchain_image_format = surfaceFormat.format;
    m_swapchain_extent = extent;
}

VulkanContext::QueueFamilyIndices 
VulkanContext::FindQueueFamilies(VkPhysicalDevice device) const
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

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present_support);

        if (present_support) {
            indices.present_family = i;
        }

        if (indices.IsComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device) const
{
    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        Swapchain::SwapChainSupportDetails swapChainSupport 
            = Swapchain::QuerySwapChainSupport(device, m_surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
    }

    return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
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

}
}