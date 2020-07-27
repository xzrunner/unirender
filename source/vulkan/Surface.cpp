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
    VkWin32SurfaceCreateInfoKHR surface_ci = {};
    surface_ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_ci.pNext = NULL;
    surface_ci.hinstance = GetModuleHandle(nullptr);
    surface_ci.hwnd = (HWND)hwnd;
    if (vkCreateWin32SurfaceKHR(m_instance->GetHandler(), &surface_ci, NULL, &m_handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create surface!");
    }
}

Surface::~Surface()
{
    vkDestroySurfaceKHR(m_instance->GetHandler(), m_handle, nullptr);
}

}
}