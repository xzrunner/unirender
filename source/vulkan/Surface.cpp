#include "unirender/vulkan/Surface.h"
#include "unirender/vulkan/Instance.h"

#include <stdexcept>

namespace ur
{
namespace vulkan
{

Surface::Surface(const std::shared_ptr<Instance>& instance, void* hwnd)
    : m_instance(instance)
{
    VkResult res = VK_ERROR_INITIALIZATION_FAILED;

#if defined(_WIN32)
    // ---- Windows (VK_KHR_win32_surface) ----
    VkWin32SurfaceCreateInfoKHR surface_ci = {};
    surface_ci.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_ci.hinstance = GetModuleHandle(nullptr);
    surface_ci.hwnd      = static_cast<HWND>(hwnd);
    res = vkCreateWin32SurfaceKHR(m_instance->GetHandler(), &surface_ci, nullptr, &m_handle);

#elif defined(VK_USE_PLATFORM_METAL_EXT) || (defined(__APPLE__) && !defined(VK_USE_PLATFORM_WAYLAND_KHR))
    // ---- macOS / iOS via MoltenVK (VK_EXT_metal_surface) ----
    // hwnd is expected to be a CAMetalLayer*
    VkMetalSurfaceCreateInfoEXT surface_ci = {};
    surface_ci.sType  = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surface_ci.pLayer = hwnd; // CAMetalLayer*
    res = vkCreateMetalSurfaceEXT(m_instance->GetHandler(), &surface_ci, nullptr, &m_handle);

#elif defined(__ANDROID__)
    // ---- Android (VK_KHR_android_surface) ----
    VkAndroidSurfaceCreateInfoKHR surface_ci = {};
    surface_ci.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surface_ci.window = static_cast<ANativeWindow*>(hwnd);
    res = vkCreateAndroidSurfaceKHR(m_instance->GetHandler(), &surface_ci, nullptr, &m_handle);

#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    // ---- Linux / Wayland (VK_KHR_wayland_surface) ----
    // hwnd is a struct { wl_display*, wl_surface* }
    struct WaylandHandles { void* display; void* surface; };
    auto* wl = static_cast<WaylandHandles*>(hwnd);
    VkWaylandSurfaceCreateInfoKHR surface_ci = {};
    surface_ci.sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surface_ci.display = static_cast<wl_display*>(wl->display);
    surface_ci.surface = static_cast<wl_surface*>(wl->surface);
    res = vkCreateWaylandSurfaceKHR(m_instance->GetHandler(), &surface_ci, nullptr, &m_handle);

#elif defined(VK_USE_PLATFORM_XCB_KHR)
    // ---- Linux / XCB (VK_KHR_xcb_surface) ----
    // hwnd is a struct { xcb_connection_t*, xcb_window_t }
    struct XcbHandles { void* connection; uint32_t window; };
    auto* xcb = static_cast<XcbHandles*>(hwnd);
    VkXcbSurfaceCreateInfoKHR surface_ci = {};
    surface_ci.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surface_ci.connection = static_cast<xcb_connection_t*>(xcb->connection);
    surface_ci.window     = xcb->window;
    res = vkCreateXcbSurfaceKHR(m_instance->GetHandler(), &surface_ci, nullptr, &m_handle);

#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    // ---- Linux / Xlib (VK_KHR_xlib_surface) ----
    // hwnd is a struct { Display*, Window }
    struct XlibHandles { void* display; unsigned long window; };
    auto* xlib = static_cast<XlibHandles*>(hwnd);
    VkXlibSurfaceCreateInfoKHR surface_ci = {};
    surface_ci.sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    surface_ci.dpy    = static_cast<Display*>(xlib->display);
    surface_ci.window = xlib->window;
    res = vkCreateXlibSurfaceKHR(m_instance->GetHandler(), &surface_ci, nullptr, &m_handle);

#else
    #error "No Vulkan surface platform defined. Define one of: _WIN32, VK_USE_PLATFORM_METAL_EXT, __ANDROID__, VK_USE_PLATFORM_XCB_KHR, VK_USE_PLATFORM_XLIB_KHR, VK_USE_PLATFORM_WAYLAND_KHR"
#endif

    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan surface!");
    }
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance->GetHandler(), m_handle, nullptr);
}

}
}
