#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>
#include <unordered_map>

namespace ur
{
namespace vulkan
{

class VulkanDevice;

class Surface;
class PhysicalDevice;
class LogicalDevice;
class CommandPool;
class CommandBuffers;
class Swapchain;
class DepthBuffer;
class UniformBuffer;
class RenderPass;
class FrameBuffers;
class DescriptorPool;
class DescriptorSet;
class PipelineCache;
class DescriptorSetLayout;
class PipelineLayout;
class Pipeline;
class ShaderProgram;
class VertexBuffer;
class IndexBuffer;

class VulkanContext
{
public:
	VulkanContext(const VulkanDevice& vk_dev);

	auto GetPhysicalDevice() const { return m_phy_dev; }
	auto GetLogicalDevice() const { return m_logic_dev; }

	void Init(uint32_t width, uint32_t height, void* hwnd);
	void Resize(uint32_t width, uint32_t height);

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	auto GetSurface() const { return m_surface; }
	auto GetSwapchain() const { return m_swapchain; }
	auto GetDepthBuffer() const { return m_depth_buf; }
	auto GetCommandBuffers() const { return m_cmd_bufs; }
	auto GetRenderPass() const { return m_renderpass; }
	auto GetFrameBuffers() const { return m_frame_buffers; }
	auto GetDescriptorSet() const { return m_desc_set; }
	auto GetPipelineLayout() const { return m_pipeline_layout; }
	auto GetPipeline() const { return m_pipeline; }
	auto GetVertexBuffer() const { return m_vert_buf; }
	auto GetShaderProgram() const { return m_program; }
	auto GetPipelineCache() const { return m_pipeline_cache; }

	void SetCurrentBuffer(uint32_t buffer) const {
		m_current_buffer = buffer;
	}
	auto GetCurrentBuffer() const { return m_current_buffer; }

private:
	const VulkanDevice& m_vk_dev;

	int m_width = 0, m_height = 0;

	std::shared_ptr<Surface> m_surface = nullptr;

	std::shared_ptr<PhysicalDevice> m_phy_dev = nullptr;
	std::shared_ptr<LogicalDevice> m_logic_dev = nullptr;

    mutable uint32_t m_current_buffer = 0;

    std::shared_ptr<CommandPool>    m_cmd_pool  = nullptr;
    std::shared_ptr<CommandBuffers> m_cmd_bufs = nullptr;

    std::shared_ptr<Swapchain>   m_swapchain = nullptr;
    std::shared_ptr<DepthBuffer> m_depth_buf = nullptr;

    std::shared_ptr<UniformBuffer> m_uniform_buf = nullptr;

    std::unordered_map<std::string, std::shared_ptr<DescriptorSetLayout>> m_desc_set_layouts;
    std::shared_ptr<PipelineLayout> m_pipeline_layout = nullptr;

    std::shared_ptr<RenderPass>   m_renderpass = nullptr;
    std::shared_ptr<FrameBuffers> m_frame_buffers = nullptr;

    std::shared_ptr<VertexBuffer>  m_vert_buf = nullptr;
    std::shared_ptr<IndexBuffer>   m_idx_buf  = nullptr;
    std::shared_ptr<ShaderProgram> m_program  = nullptr;

    std::shared_ptr<DescriptorPool> m_desc_pool = nullptr;
    std::shared_ptr<DescriptorSet>  m_desc_set  = nullptr;

    std::shared_ptr<PipelineCache> m_pipeline_cache = nullptr;
    std::shared_ptr<Pipeline>      m_pipeline = nullptr;

	bool m_include_depth = false;

	struct {
		VkDescriptorImageInfo image_info;
	} m_texture_data;

}; // VulkanContext

}
}