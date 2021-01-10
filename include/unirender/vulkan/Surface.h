#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>


namespace ur
{
namespace vulkan
{

class Instance;

class Surface : noncopyable
{
public:
	Surface(const std::shared_ptr<Instance>& instance, void* hwnd);
	~Surface();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<Instance> m_instance = nullptr;

	VkSurfaceKHR m_handle;

}; // Surface

}
}