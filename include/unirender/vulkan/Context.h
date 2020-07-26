#pragma once

#undef DrawState

#include "unirender/Context.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>

#undef DrawState

namespace ur
{

class Device;

namespace vulkan
{

class PhysicalDevice;
class LogicalDevice;
class Device;
class Surface;
class CommandPool;
class CommandBuffers;
class Swapchain;
class DepthBuffer;
class UniformBuffer;
class RenderPass;
class FrameBuffers;
class DescriptorSet;
class PipelineCache;
class DescriptorSetLayout;
class PipelineLayout;
class Pipeline;
class ShaderProgram;
class VertexBuffer;
class IndexBuffer;

class Context : public ur::Context
{
public:
    Context(const ur::Device& device, void* hwnd, 
        uint32_t width, uint32_t height);
    virtual ~Context();

    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual void Clear(const ClearState& clear_state) override;
    virtual void Draw(PrimitiveType prim_type, int offset, int count,
        const DrawState& draw, const void* scene) override;
    virtual void Draw(PrimitiveType prim_type, const DrawState& draw,
        const void* scene) override;

    virtual void SetViewport(int x, int y, int w, int h) override;
    virtual void GetViewport(int& x, int& y, int& w, int& h) const override;

    virtual void SetTexture(size_t slot, const ur::TexturePtr& tex) override;
    virtual void SetTextureSampler(size_t slot, const std::shared_ptr<ur::TextureSampler>& sampler) override;
	virtual void SetFramebuffer(const std::shared_ptr<ur::Framebuffer>& fb) override;
	virtual std::shared_ptr<ur::Framebuffer> GetFramebuffer() const override;

    virtual void SetUnpackRowLength(int len) override;
    virtual void SetPackRowLength(int len) override;

    virtual bool CheckRenderTargetStatus() override;

    virtual void Flush() override;

    void Init(void* hwnd, uint32_t width, uint32_t height);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    std::shared_ptr<PhysicalDevice> GetPhysicalDevice() const;
    std::shared_ptr<LogicalDevice> GetLogicalDevice() const;
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
	void Draw();

    void BuildCommandBuffers();

private:
    const Device& m_device;

    VkSemaphore presentCompleteSemaphore;
    VkSemaphore renderCompleteSemaphore;

    std::vector<VkFence> waitFences;

	int m_width = 0, m_height = 0;

	std::shared_ptr<Surface> m_surface = nullptr;

    mutable uint32_t m_current_buffer = 0;

    std::shared_ptr<CommandPool>    m_cmd_pool  = nullptr;
    std::shared_ptr<CommandBuffers> m_cmd_bufs = nullptr;

    std::shared_ptr<Swapchain>   m_swapchain = nullptr;
    std::shared_ptr<DepthBuffer> m_depth_buf = nullptr;

    std::shared_ptr<UniformBuffer> m_uniform_buf = nullptr;

    std::shared_ptr<PipelineLayout> m_pipeline_layout = nullptr;

    std::shared_ptr<RenderPass>   m_renderpass = nullptr;
    std::shared_ptr<FrameBuffers> m_frame_buffers = nullptr;

    std::shared_ptr<VertexBuffer>  m_vert_buf = nullptr;
    std::shared_ptr<IndexBuffer>   m_idx_buf  = nullptr;
    std::shared_ptr<ShaderProgram> m_program  = nullptr;

    std::shared_ptr<DescriptorPool> m_desc_pool = nullptr;
    std::shared_ptr<DescriptorSet>  m_desc_set = nullptr;

    std::shared_ptr<PipelineCache> m_pipeline_cache = nullptr;
    std::shared_ptr<Pipeline>      m_pipeline = nullptr;

	bool m_include_depth = false;

	//struct {
	//	VkDescriptorImageInfo image_info;
	//} m_texture_data;
    std::shared_ptr<Texture> m_texture = nullptr;

}; // Context

}
}