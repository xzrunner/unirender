#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;

class DescriptorPool : boost::noncopyable
{
public:
	DescriptorPool(const std::shared_ptr<LogicalDevice>& device, bool use_texture);
	~DescriptorPool();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkDescriptorPool m_handle = VK_NULL_HANDLE;

}; // DescriptorPool

}
}