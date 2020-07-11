#pragma once

#include <vulkan/vulkan.h>
#include <glm/matrix.hpp>

namespace ur
{
namespace vulkan
{

class DeviceInfo;
class ContextInfo;

class UniformBuffer
{
public:
	UniformBuffer(VkDevice device);
	~UniformBuffer();

	void Create(const DeviceInfo& dev_info,
		const ContextInfo& ctx_info);

	auto GetBufferInfo() const { return m_buffer_info; }

private:
	VkDevice m_device;

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