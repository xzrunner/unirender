#pragma once

#include "unirender/Pipeline.h"
#include "unirender/Blending.h"

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
	// Overload without an explicit pipeline layout: the layout is taken from the
	// program's reflection (used when the engine doesn't pre-build a pipeline, e.g.
	// easygui's RenderBuffer::Draw which passes only program + vertex_array).
	Pipeline(const Context& ctx, bool include_depth, bool include_vi,
		const ur::VertexBuffer& vb, const ur::ShaderProgram& prog,
		const Blending& blend = Blending(),
		VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	~Pipeline();

	auto GetHandler() const { return m_handle; }

private:
	void Build(const Context& ctx, bool include_depth, bool include_vi,
		const ur::VertexBuffer& vb, const ur::ShaderProgram& prog,
		const Blending& blend, VkPrimitiveTopology topology);

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkPipeline m_handle;

}; // Pipeline

}
}