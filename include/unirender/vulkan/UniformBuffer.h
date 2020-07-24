#pragma once

#include <vulkan/vulkan.h>
#include <glm/matrix.hpp>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;

class UniformBuffer : boost::noncopyable
{
public:
	UniformBuffer(const std::shared_ptr<LogicalDevice>& device,
		const PhysicalDevice& phy_dev, int width, int height);
	~UniformBuffer();

	auto GetBufferInfo() const { return m_buffer_info; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkBuffer m_buf;
	VkDeviceMemory m_mem;
	VkDescriptorBufferInfo m_buffer_info;

	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;
	glm::mat4 Clip;
	glm::mat4 MVP;

}; // UniformBuffer

}
}