#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class Instance : noncopyable
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