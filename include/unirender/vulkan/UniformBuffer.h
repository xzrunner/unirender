#pragma once

#include <vulkan/vulkan.h>
#include <glm/matrix.hpp>

namespace ur
{
namespace vulkan
{

class VulkanContext;

class UniformBuffer
{
public:
	UniformBuffer(VkDevice device);
	~UniformBuffer();

	void Create(const VulkanContext& vk_ctx);

	auto GetBufferInfo() const { return m_buffer_info; }

private:
	VkDevice m_device = VK_NULL_HANDLE;

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