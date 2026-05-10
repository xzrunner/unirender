#pragma once

#undef DrawState

#include "unirender/Context.h"
#include "unirender/ClearState.h"
#include "unirender/Rectangle.h"
#include "unirender/typedef.h"

#include <vulkan/vulkan.h>

#include <array>
#include <memory>
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
    virtual void SetImage(size_t slot, const ur::TexturePtr& tex, AccessType access) override;

    virtual void SetFramebuffer(const std::shared_ptr<ur::Framebuffer>& fb) override;
    virtual std::shared_ptr<ur::Framebuffer> GetFramebuffer() const override;

    virtual void SetUnpackRowLength(int len) override;
    virtual void SetPackRowLength(int len) override;

    virtual bool CheckRenderTargetStatus() override;

    virtual void Flush() override;

    virtual std::shared_ptr<ur::Pipeline> CreatePipeline(bool include_depth, bool include_vi,
        const ur::PipelineLayout& layout, const ur::VertexBuffer& vb,
        const ur::ShaderProgram& prog) const override;

    virtual void SetMemoryBarrier(const std::vector<BarrierType>& types) override;

    // Accessors used by RenderPass, FrameBuffers, Pipeline, etc.
    void Init(void* hwnd, uint32_t width, uint32_t height);

    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }

    std::shared_ptr<PhysicalDevice> GetPhysicalDevice() const;
    std::shared_ptr<LogicalDevice>  GetLogicalDevice()  const;

    auto GetSurface()      const { return m_surface; }
    auto GetSwapchain()    const { return m_swapchain; }
    auto GetDepthBuffer()  const { return m_depth_buf; }
    auto GetCommandBuffer()const { return m_cmd_buf; }
    auto GetRenderPass()   const { return m_renderpass; }
    auto GetFrameBuffers() const { return m_frame_buffers; }
    auto GetPipelineCache()const { return m_pipeline_cache; }

    void SetCurrentBuffer(uint32_t buffer) const { m_current_buffer = buffer; }
    auto GetCurrentBuffer() const { return m_current_buffer; }

private:
    // Core draw implementation -- full acquire/build/submit/present pipeline
    void DrawImpl(const DrawState& draw, PrimitiveType prim_type,
                  int offset, int count);

    // Record commands into m_cmd_buf
    void BuildCommandBuffer(const DrawState& draw, PrimitiveType prim_type,
                            int offset, int count);

    // Wait for previous frame's fence
    void WaitSync();

    // Setup commonly used descriptor set layouts on the Device
    void SetupDescriptorLayouts();

private:
    const Device& m_device;

    int m_width  = 0;
    int m_height = 0;

    // Clear state
    ClearBuffers m_clear_flag;
    Color  m_clear_color   = Color(0, 0, 0, 0);
    double m_clear_depth   = 1.0;
    int    m_clear_stencil = 0;

    Rectangle m_viewport;

    // Synchronisation
    struct {
        VkSemaphore present_complete = VK_NULL_HANDLE;
        VkSemaphore render_complete  = VK_NULL_HANDLE;
    } m_semaphores;
    VkFence m_wait_fence = VK_NULL_HANDLE;

    // Vulkan objects
    std::shared_ptr<Surface>       m_surface       = nullptr;
    std::shared_ptr<Swapchain>     m_swapchain     = nullptr;
    std::shared_ptr<DepthBuffer>   m_depth_buf     = nullptr;
    std::shared_ptr<CommandPool>   m_cmd_pool      = nullptr;
    std::shared_ptr<CommandBuffer> m_cmd_buf       = nullptr;
    std::shared_ptr<RenderPass>    m_renderpass    = nullptr;
    std::shared_ptr<FrameBuffers>  m_frame_buffers = nullptr;
    std::shared_ptr<PipelineCache> m_pipeline_cache= nullptr;

    mutable uint32_t m_current_buffer = 0;
    bool m_include_depth = false;

    // Framebuffer override (off-screen)
    std::shared_ptr<ur::Framebuffer> m_set_framebuffer = nullptr;

    // Bound texture/sampler slots (for next draw call)
    static constexpr size_t MAX_TEXTURE_SLOTS = 32;
    std::array<ur::TexturePtr, MAX_TEXTURE_SLOTS> m_bound_textures = {};
    std::array<std::shared_ptr<ur::TextureSampler>, MAX_TEXTURE_SLOTS> m_bound_samplers = {};

}; // Context

}
}
