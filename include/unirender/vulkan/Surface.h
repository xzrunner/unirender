#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class Surface
{
public:
	Surface(VkInstance instance);
	~Surface();

	void Create(void* hwnd);

	auto GetHandler() const { return m_handle; }

private:
	VkInstance m_instance;

	VkSurfaceKHR m_handle;

}; // Surface

}
}