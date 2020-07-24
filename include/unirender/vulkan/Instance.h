#pragma once

#include <vulkan/vulkan.h>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class Instance : boost::noncopyable
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