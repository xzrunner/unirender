#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class PhysicalDevice;
class Surface;

class LogicalDevice : boost::noncopyable
{
public:
	LogicalDevice(bool enable_validation_layers, 
		const PhysicalDevice& phy_dev, const Surface* surface = nullptr);
	~LogicalDevice();

	auto GetHandler() const { return m_handle; }

	auto GetGraphicsQueue() const { return m_graphics_queue; }
	auto GetPresentQueue() const { return m_present_queue; }

private:
	VkDevice m_handle = VK_NULL_HANDLE;

	VkQueue m_graphics_queue = VK_NULL_HANDLE;
	VkQueue m_present_queue  = VK_NULL_HANDLE;

}; // LogicalDevice

}
}