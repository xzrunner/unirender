#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace ur
{
namespace vulkan
{

class DeviceInfo;
class ContextInfo;

class DescriptorSet
{
public:
	DescriptorSet(VkDevice device);
	~DescriptorSet();

	void Create(const DeviceInfo& dev_info,
		const ContextInfo& ctx_info, bool use_texture);

	auto& GetHandler() const { return m_handle; }

private:
	VkDevice m_device;

	std::vector<VkDescriptorSet> m_handle;

}; // DescriptorSet

}
}