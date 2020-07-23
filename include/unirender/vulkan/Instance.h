#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class Instance
{
public:
	Instance(bool enable_validation_layers);
	~Instance();

	auto GetHandler() const { return m_handle; }

private:
	VkInstance m_handle;

}; // Instance

}
}