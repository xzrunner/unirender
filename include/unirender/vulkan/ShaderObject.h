#pragma once

#include "unirender/ShaderType.h"

#include <vulkan/vulkan.h>

#include <string>

namespace ur
{
namespace vulkan
{

class ShaderObject
{
public:
	ShaderObject(VkDevice dev, ShaderType type, const uint32_t* code, size_t code_sz);
	~ShaderObject();

	auto GetHandler() const { return m_stage; }

private:
	VkDevice m_dev;

	VkPipelineShaderStageCreateInfo m_stage;

}; // ShaderObject

}
}