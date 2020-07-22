#include "unirender/vulkan/Surface.h"

#include <stdexcept>

namespace ur
{
namespace vulkan
{

Surface::Surface(VkInstance instance)
    : m_instance(instance)
{
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance, m_handle, nullptr);
}

void Surface::Create(void* hwnd)
{
    VkWin32SurfaceCreateInfoKHR surface_ci = {};
    surface_ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_ci.pNext = NULL;
    surface_ci.hinstance = GetModuleHandle(nullptr);
    surface_ci.hwnd = (HWND)hwnd;
    if (vkCreateWin32SurfaceKHR(m_instance, &surface_ci, NULL, &m_handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create m_handle!");
    }
}

}
}