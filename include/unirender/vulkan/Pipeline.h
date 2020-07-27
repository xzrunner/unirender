#pragma once

#include "unirender/Pipeline.h"

#include <vulkan/vulkan.h>

#include <memory>

namespace ur
{

class PipelineLayout;
class VertexBuffer;
class ShaderProgram;

namespace vulkan
{

class Context;
class LogicalDevice;

class Pipeline : public ur::Pipeline
{
public:
	Pipeline(const Context& ctx, bool include_depth, bool include_vi,
		const ur::PipelineLayout& layout, const ur::VertexBuffer& vb,
		const ur::ShaderProgram& prog);
	~Pipeline();

	auto GetHandler() const { return m_handle; }

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkPipeline m_handle;

}; // Pipeline

}
}