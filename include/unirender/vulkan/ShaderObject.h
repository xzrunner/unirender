#pragma once

#include "unirender/ShaderType.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;

class ShaderObject : boost::noncopyable
{
public:
	ShaderObject(const std::shared_ptr<LogicalDevice>& device, 
		ShaderType type, const std::vector<unsigned int>& spirv);
	~ShaderObject();

	auto GetHandler() const { return m_handle; }

private:
	void Init(ShaderType type, const std::vector<unsigned int>& spirv);

private:
	std::shared_ptr<LogicalDevice> m_device = VK_NULL_HANDLE;

	VkPipelineShaderStageCreateInfo m_handle;

}; // ShaderObject

}
}