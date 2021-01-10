#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>

namespace ur
{
namespace vulkan
{

class LogicalDevice;

class Buffer : noncopyable
{
public:
	Buffer(const std::shared_ptr<LogicalDevice>& device);
	~Buffer();

	void Create(VkPhysicalDevice phy_dev, VkDeviceSize size, 
		VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

	void Upload(const void* data, size_t size);
	void CopyFrom(const Buffer& src, size_t size, 
		VkCommandPool cmd_pool, VkQueue graphics_queue);

	size_t GetBufferSize() const { return m_size; }

	auto GetHandler() const { return m_buffer; }

private:
	void Clear();

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkBuffer       m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;

	size_t m_size = 0;

}; // Buffer

}
}