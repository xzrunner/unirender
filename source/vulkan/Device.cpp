#include "unirender/vulkan/Device.h"

#include <SM_Vector.h>

#include <iostream>

#include <assert.h>

namespace ur
{
namespace vulkan
{

Device::Device()
{
    Init();
}

std::shared_ptr<ur::VertexArray>
Device::GetVertexArray(PrimitiveType prim, VertexLayoutType layout) const
{
    return nullptr;
}

std::shared_ptr<ur::VertexArray>
Device::CreateVertexArray() const
{
    return nullptr;
}

std::shared_ptr<ur::Framebuffer>
Device::CreateFramebuffer() const
{
    return nullptr;
}

std::shared_ptr<ur::RenderBuffer>
Device::CreateRenderBuffer(int width, int height, InternalFormat format, AttachmentType attach) const
{
    return nullptr;
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::string& vs, const std::string& fs, const std::string& gs,
                            const std::vector<std::string>& attr_names) const
{
    return nullptr;
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::string& cs) const
{
    return nullptr;
}

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
    return nullptr;
}

std::shared_ptr<ur::IndexBuffer>
Device::CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
    return nullptr;
}

std::shared_ptr<ur::WritePixelBuffer>
Device::CreateWritePixelBuffer(BufferUsageHint hint, int size_in_bytes) const
{
    return nullptr;
}

std::shared_ptr<ur::ComputeBuffer>
Device::CreateComputeBuffer(const std::vector<int>& buf, size_t index) const
{
    return nullptr;
}

std::shared_ptr<ur::ComputeBuffer>
Device::CreateComputeBuffer(const std::vector<float>& buf, size_t index) const
{
    return nullptr;
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(const TextureDescription& desc, const void* pixels) const
{
    return nullptr;
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(size_t width, size_t height, TextureFormat format, const void* buf, size_t buf_sz) const
{
    return nullptr;
}

std::shared_ptr<ur::TextureSampler>
Device::CreateTextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter, TextureWrap wrap_s, TextureWrap wrap_t) const
{
    return nullptr;
}

std::shared_ptr<ur::TextureSampler>
Device::GetTextureSampler(TextureSamplerType type) const
{
    return nullptr;
}

void Device::DispatchCompute(int thread_group_count) const
{
}

void Device::ReadPixels(const unsigned char* pixels, ur::TextureFormat format,
                        int x, int y, int w, int h) const
{
}

void Device::ReadPixels(const short* pixels, ur::TextureFormat format,
                        int x, int y, int w, int h) const
{
}

void Device::Init()
{
    InitGlobalLayerProperties();
    InitInstanceExtensionNames();
    InitDeviceExtensionNames();
    InitInstance("unirender-vulkan");
    InitEnumerateDevice();
    InitDevice();
}

VkResult Device::InitGlobalLayerProperties()
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
        m_instance_layer_properties.push_back(layer_props);
    }
    free(vk_props);

    return res;
}

void Device::InitInstanceExtensionNames()
{
    m_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef __ANDROID__
    m_instance_extension_names.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32)
    m_instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    m_instance_extension_names.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    m_instance_extension_names.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
    m_instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
}

void Device::InitDeviceExtensionNames()
{
    m_device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VkResult Device::InitInstance(const char* title)
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = title;
    app_info.applicationVersion = 1;
    app_info.pEngineName = title;
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledLayerCount = m_instance_layer_names.size();
    inst_info.ppEnabledLayerNames = m_instance_layer_names.size() ? m_instance_layer_names.data() : NULL;
    inst_info.enabledExtensionCount = m_instance_extension_names.size();
    inst_info.ppEnabledExtensionNames = m_instance_extension_names.data();

    VkResult res = vkCreateInstance(&inst_info, NULL, &m_inst);
    assert(res == VK_SUCCESS);

    return res;
}

VkResult Device::InitEnumerateDevice(uint32_t gpu_count)
{
    uint32_t const req_count = gpu_count;
    VkResult res = vkEnumeratePhysicalDevices(m_inst, &gpu_count, NULL);
    assert(gpu_count);
    m_gpus.resize(gpu_count);

    res = vkEnumeratePhysicalDevices(m_inst, &gpu_count, m_gpus.data());
    assert(!res && gpu_count >= req_count);

    vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[0], &m_queue_family_count, NULL);
    assert(m_queue_family_count >= 1);

    m_queue_props.resize(m_queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[0], &m_queue_family_count, m_queue_props.data());
    assert(m_queue_family_count >= 1);

    /* This is as good a place as any to do this */
    vkGetPhysicalDeviceMemoryProperties(m_gpus[0], &m_memory_properties);
    vkGetPhysicalDeviceProperties(m_gpus[0], &m_gpu_props);
    /* query device extensions for enabled layers */
    for (auto &layer_props : m_instance_layer_properties) {
        InitDeviceExtensionProperties(layer_props);
    }

    return res;
}

void Device::InitSwapchainExtension()
{
//    /* DEPENDS on init_connection() and init_window() */
//
//    VkResult res;
//
//// Construct the surface description:
//#ifdef _WIN32
//    VkWin32SurfaceCreateInfoKHR createInfo = {};
//    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
//    createInfo.pNext = NULL;
//    createInfo.hinstance = m_connection;
//    createInfo.hwnd = m_window;
//    res = vkCreateWin32SurfaceKHR(m_inst, &createInfo, NULL, &m_surface);
//#elif defined(__ANDROID__)
//    GET_INSTANCE_PROC_ADDR(m_inst, CreateAndroidSurfaceKHR);
//
//    VkAndroidSurfaceCreateInfoKHR createInfo;
//    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
//    createInfo.pNext = nullptr;
//    createInfo.flags = 0;
//    createInfo.window = AndroidGetApplicationWindow();
//    res = m_fpCreateAndroidSurfaceKHR(m_inst, &createInfo, nullptr, &m_surface);
//#elif defined(VK_USE_PLATFORM_METAL_EXT)
//    VkMetalSurfaceCreateInfoEXT createInfo = {};
//    createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
//    createInfo.pNext = NULL;
//    createInfo.flags = 0;
//    createInfo.pLayer = m_caMetalLayer;
//    res = vkCreateMetalSurfaceEXT(m_inst, &createInfo, NULL, &m_surface);
//#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
//    VkWaylandSurfaceCreateInfoKHR createInfo = {};
//    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
//    createInfo.pNext = NULL;
//    createInfo.display = m_display;
//    createInfo.surface = m_window;
//    res = vkCreateWaylandSurfaceKHR(m_inst, &createInfo, NULL, &m_surface);
//#else
//    VkXcbSurfaceCreateInfoKHR createInfo = {};
//    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
//    createInfo.pNext = NULL;
//    createInfo.connection = m_connection;
//    createInfo.window = m_window;
//    res = vkCreateXcbSurfaceKHR(m_inst, &createInfo, NULL, &m_surface);
//#endif  // __ANDROID__  && _WIN32
//    assert(res == VK_SUCCESS);
//
//    // Iterate over each queue to learn whether it supports presenting:
//    VkBool32 *pSupportsPresent = (VkBool32 *)malloc(m_queue_family_count * sizeof(VkBool32));
//    for (uint32_t i = 0; i < m_queue_family_count; i++) {
//        vkGetPhysicalDeviceSurfaceSupportKHR(m_gpus[0], i, m_surface, &pSupportsPresent[i]);
//    }
//
//    // Search for a graphics and a present queue in the array of queue
//    // families, try to find one that supports both
//    m_graphics_queue_family_index = UINT32_MAX;
//    m_present_queue_family_index = UINT32_MAX;
//    for (uint32_t i = 0; i < m_queue_family_count; ++i) {
//        if ((m_queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
//            if (m_graphics_queue_family_index == UINT32_MAX) m_graphics_queue_family_index = i;
//
//            if (pSupportsPresent[i] == VK_TRUE) {
//                m_graphics_queue_family_index = i;
//                m_present_queue_family_index = i;
//                break;
//            }
//        }
//    }
//
//    if (m_present_queue_family_index == UINT32_MAX) {
//        // If didn't find a queue that supports both graphics and present, then
//        // find a separate present queue.
//        for (size_t i = 0; i < m_queue_family_count; ++i)
//            if (pSupportsPresent[i] == VK_TRUE) {
//                m_present_queue_family_index = i;
//                break;
//            }
//    }
//    free(pSupportsPresent);
//
//    // Generate error if could not find queues that support graphics
//    // and present
//    if (m_graphics_queue_family_index == UINT32_MAX || m_present_queue_family_index == UINT32_MAX) {
//        std::cout << "Could not find a queues for both graphics and present";
//        exit(-1);
//    }
//
//    // Get the list of VkFormats that are supported:
//    uint32_t formatCount;
//    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[0], m_surface, &formatCount, NULL);
//    assert(res == VK_SUCCESS);
//    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
//    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpus[0], m_surface, &formatCount, surfFormats);
//    assert(res == VK_SUCCESS);
//    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
//    // the surface has no preferred format.  Otherwise, at least one
//    // supported format will be returned.
//    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
//        m_format = VK_FORMAT_B8G8R8A8_UNORM;
//    } else {
//        assert(formatCount >= 1);
//        m_format = surfFormats[0].format;
//    }
//    free(surfFormats);
}

VkResult Device::InitDevice()
{
    VkResult res;
    VkDeviceQueueCreateInfo queue_info = {};

    float queue_priorities[1] = {0.0};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = queue_priorities;
    queue_info.queueFamilyIndex = m_graphics_queue_family_index;

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = NULL;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = m_device_extension_names.size();
    device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount ? m_device_extension_names.data() : NULL;
    device_info.pEnabledFeatures = NULL;

    res = vkCreateDevice(m_gpus[0], &device_info, NULL, &m_device);
    assert(res == VK_SUCCESS);

    return res;
}

VkResult Device::InitGlobalExtensionProperties(layer_properties& layer_props)
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

VkResult Device::InitDeviceExtensionProperties(layer_properties& layer_props)
{
    VkExtensionProperties *device_extensions;
    uint32_t device_extension_count;
    VkResult res;
    char *layer_name = NULL;

    layer_name = layer_props.properties.layerName;

    do {
        res = vkEnumerateDeviceExtensionProperties(m_gpus[0], layer_name, &device_extension_count, NULL);
        if (res) return res;

        if (device_extension_count == 0) {
            return VK_SUCCESS;
        }

        layer_props.device_extensions.resize(device_extension_count);
        device_extensions = layer_props.device_extensions.data();
        res = vkEnumerateDeviceExtensionProperties(m_gpus[0], layer_name, &device_extension_count, device_extensions);
    } while (res == VK_INCOMPLETE);

    return res;
}

}
}