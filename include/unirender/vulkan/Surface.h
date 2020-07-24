#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class Instance;

class Surface : boost::noncopyable
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