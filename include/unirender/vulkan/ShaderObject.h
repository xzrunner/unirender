#pragma once

#include "unirender/ShaderType.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace ur
{
namespace vulkan
{

class ShaderObject
{
public:
	ShaderObject(VkDevice dev, ShaderType type, const std::vector<unsigned int>& spirv);
	~ShaderObject();

	auto GetHandler() const { return m_stage; }

private:
	void Init(ShaderType type, const std::vector<unsigned int>& spirv);

private:
	VkDevice m_dev;

	VkPipelineShaderStageCreateInfo m_stage;

}; // ShaderObject

}
}