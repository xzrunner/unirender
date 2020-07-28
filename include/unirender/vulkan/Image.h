#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;

class Image : boost::noncopyable
{
public:
	Image(const std::shared_ptr<LogicalDevice>& device, const PhysicalDevice& phy_dev,
		VkImageType type, VkFormat format, VkExtent3D extent, uint32_t mip_levels = 1, uint32_t array_layers = 1);
	~Image();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkImage m_handle;

	VkDeviceMemory m_memory;

}; // Image

}
}