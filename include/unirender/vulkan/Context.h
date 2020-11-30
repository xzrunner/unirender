#pragma once

#undef DrawState

#include "unirender/Context.h"
#include "unirender/ClearState.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>

#undef DrawState

namespace ur
{

class Device;
class DescriptorSet;

namespace vulkan
{

class PhysicalDevice;
class LogicalDevice;
class Device;
class Surface;
class CommandPool;
class CommandBuffer;
class Swapchain;
class DepthBuffer;
class UniformBuffer;
class RenderPass;
class FrameBuffers;
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
    virtual void Compute(const DrawState& draw, int num_groups_x, 
        int num_groups_y, int num_groups_z) override {}

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

    virtual std::shared_ptr<ur::Pipeline> CreatePipeline(bool include_depth, bool include_vi, const ur::PipelineLayout& layout,
        const ur::VertexBuffer& vb, const ur::ShaderProgram& prog) const override;

    virtual void MemoryBarrier(const std::vector<BarrierType>& types) override {}

    void Init(void* hwnd, uint32_t width, uint32_t height);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    std::shared_ptr<PhysicalDevice> GetPhysicalDevice() const;
    std::shared_ptr<LogicalDevice> GetLogicalDevice() const;
	auto GetSurface() const { return m_surface; }
	auto GetSwapchain() const { return m_swapchain; }
	auto GetDepthBuffer() const { return m_depth_buf; }
	auto GetCommandBuffers() const { return m_cmd_buf; }
	auto GetRenderPass() const { return m_renderpass; }
	auto GetFrameBuffers() const { return m_frame_buffers; }
	auto GetPipelineCache() const { return m_pipeline_cache; }

	void SetCurrentBuffer(uint32_t buffer) const {
		m_current_buffer = buffer;
	}
	auto GetCurrentBuffer() const { return m_current_buffer; }

private:
	void Draw(const DrawState& draw);

    void BuildCommandBuffers(const DrawState& draw);

    void WaitSync();

private:
    const Device& m_device;

    int m_width = 0, m_height = 0;

    ClearBuffers m_clear_flag;
    Color  m_clear_color   = Color(0, 0, 0, 0);
    double m_clear_depth   = 0;
    int    m_clear_stencil = 0;
    Rectangle m_viewport;

    struct {
        VkSemaphore present_complete;
        VkSemaphore render_complete;
    } m_semaphores;

    VkFence m_wait_fence;

	std::shared_ptr<Surface> m_surface = nullptr;

    mutable uint32_t m_current_buffer = 0;

    std::shared_ptr<CommandPool>   m_cmd_pool = nullptr;
    std::shared_ptr<CommandBuffer> m_cmd_buf  = nullptr;

    std::shared_ptr<Swapchain>   m_swapchain = nullptr;
    std::shared_ptr<DepthBuffer> m_depth_buf = nullptr;

    std::shared_ptr<RenderPass>   m_renderpass = nullptr;
    std::shared_ptr<FrameBuffers> m_frame_buffers = nullptr;

    std::shared_ptr<PipelineCache> m_pipeline_cache = nullptr;

	bool m_include_depth = false;

}; // Context

}
}