#pragma once

#include "unirender/DescriptorSet.h"
#include "unirender/DescriptorType.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{

class UniformBuffer;
class Texture;
class DescriptorSetLayout;

namespace vulkan
{

class LogicalDevice;
class DescriptorPool;

class DescriptorSet : public ur::DescriptorSet
{
public:
	DescriptorSet(const std::shared_ptr<LogicalDevice>& device, const DescriptorPool& pool,
		const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& layouts,
		const std::vector<ur::DescriptorSet::Descriptor>& descriptors);
	~DescriptorSet();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkDescriptorSet m_handle = VK_NULL_HANDLE;

}; // DescriptorSet

}
}