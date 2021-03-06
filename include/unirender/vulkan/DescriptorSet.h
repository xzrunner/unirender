#pragma once

#include "unirender/DescriptorSet.h"
#include "unirender/DescriptorType.h"
#include "unirender/Descriptor.h"
#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>


namespace ur
{

class UniformBuffer;
class Texture;
class DescriptorSetLayout;
class DescriptorPool;
struct Descriptor;
class Device;

namespace vulkan
{

class LogicalDevice;

class DescriptorSet : public ur::DescriptorSet
{
public:
	DescriptorSet(const ur::Device& dev, const std::shared_ptr<LogicalDevice>& device, const ur::DescriptorPool& pool,
		const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& layouts,
		const std::vector<ur::Descriptor>& descriptors);
	~DescriptorSet();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkDescriptorSet m_handle = VK_NULL_HANDLE;

}; // DescriptorSet

}
}