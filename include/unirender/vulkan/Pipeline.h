#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class Context;
class LogicalDevice;

class Pipeline : boost::noncopyable
{
public:
	Pipeline(const Context& ctx, bool include_depth, bool include_vi);
	~Pipeline();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkPipeline m_handle;

}; // Pipeline

}
}