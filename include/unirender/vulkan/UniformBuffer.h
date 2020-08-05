#pragma once

#include "unirender/UniformBuffer.h"
#include "unirender/vulkan/Buffer.h"

#include <vulkan/vulkan.h>

#include <memory>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class PhysicalDevice;

class UniformBuffer : public ur::UniformBuffer
{
public:
	UniformBuffer(const std::shared_ptr<LogicalDevice>& device,
		const PhysicalDevice& phy_dev, const void* data, size_t size);

	virtual void Update(const void* data, size_t size) override;

	auto& GetBufferInfo() const { return m_buffer_info; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	Buffer m_buffer;
	VkDescriptorBufferInfo m_buffer_info;

}; // UniformBuffer

}
}