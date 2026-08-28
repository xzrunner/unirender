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
#include <map>
#include <tuple>
#include <utility>
#include <cstdint>

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
class DescriptorPool;
class Swapchain;
class DepthBuffer;
class UniformBuffer;
class RenderPass;
class FrameBuffers;
class Framebuffer;
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

    virtual void InvalidateCachedState() override {}
    virtual void NotifyExternalStateMutation() override {}
    virtual void CommitFramebuffer() override {}
    virtual void CommitViewport() override {}
    virtual void CommitBindings() override {}

    // Accessors used by RenderPass, FrameBuffers, Pipeline, etc.
    void Init(void* hwnd, uint32_t width, uint32_t height);

    int GetWidth()  const { return m_width; }
    int GetHeight() const { return m_height; }

    std::shared_ptr<PhysicalDevice> GetPhysicalDevice() const;
    std::shared_ptr<LogicalDevice>  GetLogicalDevice()  const;

    auto GetSurface()      const { return m_surface; }
    auto GetSwapchain()    const { return m_swapchain; }
    auto GetDepthBuffer()  const { return m_depth_buf; }
    auto GetCommandBuffer()const { return m_frames[m_frame_slot].cmd_buf; }
    auto GetRenderPass()   const { return m_renderpass; }
    auto GetFrameBuffers() const { return m_frame_buffers; }
    auto GetPipelineCache()const { return m_pipeline_cache; }

    // The VkRenderPass of the pass currently being recorded (screen or offscreen).
    // Pipelines must be created against THIS so they're render-pass-compatible with
    // wherever they're used (a screen-pass pipeline can't draw in an offscreen pass).
    VkRenderPass GetCurrentRenderPass() const { return m_cur_render_pass; }
    // Color attachment count of the current pass (MRT GBuffer = 3, screen = 1). The
    // pipeline's color-blend state must declare exactly this many attachments.
    uint32_t GetCurrentColorAttachmentCount() const { return m_cur_color_count; }

    void SetCurrentBuffer(uint32_t buffer) const { m_current_buffer = buffer; }
    auto GetCurrentBuffer() const { return m_current_buffer; }

private:
    // Core draw implementation -- records into the current frame's command buffer.
    void DrawImpl(const DrawState& draw, PrimitiveType prim_type,
                  int offset, int count);

    // Record ONE draw (uniforms, descriptor set, pipeline, vertex/index, draw)
    // into `cmd_buf`. Does NOT begin/end the command buffer or the render pass.
    void RecordDraw(VkCommandBuffer cmd_buf, const DrawState& draw,
                    PrimitiveType prim_type, int offset, int count);

    // Frame batching: a whole frame is recorded into one command buffer and
    // presented once (in Flush), mirroring the Metal BeginFrame/EndFrame model.
    // Per Draw we only switch to the right render pass (screen vs offscreen FBO).
    void BeginFrameIfNeeded();
    void BeginPass(const vulkan::Framebuffer* fbo); // null = screen
    void EndCurrentPass();

    // Immediately clear an offscreen FBO's color attachments (the GBuffer's per-frame
    // background); the screen clear stays deferred to the first screen pass.
    void ClearFboColor(const vulkan::Framebuffer& fbo, const Color& color);

    // Diagnostic: copy a VkImage to a PPM file (gated by env TT_VK_DUMP). Used to
    // inspect what the offscreen render target / swapchain actually contain.
    void DumpImageToPPM(VkImage image, VkFormat format, uint32_t w, uint32_t h,
                        VkImageLayout cur_layout, const char* path);

    // Setup commonly used descriptor set layouts on the Device
    void SetupDescriptorLayouts();

    // Frames-in-flight ring lifecycle.
    void CreateFrameResources();   // per-frame cmd buffer / fence / acquire-sem / desc pool
    void DestroyFrameResources();
    void CreateRenderFinishedSemaphores();  // one per swapchain image (resize-dependent)
    void DestroyRenderFinishedSemaphores();
    // Handle of the command buffer the current frame is recording into.
    VkCommandBuffer CurCmd() const;

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

    // ---- Frames-in-flight synchronisation -----------------------------------
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    // One independent set of GPU resources per in-flight frame. The CPU records
    // frame N+1 into slot (N+1)%MAX while the GPU still executes frame N from slot
    // N%MAX, so CPU and GPU overlap instead of lock-stepping a submit+wait per draw.
    struct FrameSlot {
        std::shared_ptr<CommandBuffer>  cmd_buf;                  // the whole frame is recorded here
        VkFence        in_flight       = VK_NULL_HANDLE;          // signalled when this slot's submit completes
        VkSemaphore    image_available = VK_NULL_HANDLE;          // signalled by vkAcquireNextImageKHR
        std::shared_ptr<DescriptorPool> desc_pool;               // per-draw sets, reset once per frame
    };
    std::array<FrameSlot, MAX_FRAMES_IN_FLIGHT> m_frames;

    // render-finished is per SWAPCHAIN IMAGE (signalled by the frame submit, waited
    // by present). A given image's semaphore is only reused once its present has
    // completed, which the presentation engine guarantees before re-acquiring it --
    // so this avoids the per-frame-semaphore reuse hazard.
    std::vector<VkSemaphore> m_render_finished;

    uint64_t m_frame_number = 0; // monotonic CPU frame counter (drives slot + retirement)
    uint32_t m_frame_slot   = 0; // m_frame_number % MAX_FRAMES_IN_FLIGHT

    // Vulkan objects
    std::shared_ptr<Surface>       m_surface       = nullptr;
    std::shared_ptr<Swapchain>     m_swapchain     = nullptr;
    std::shared_ptr<DepthBuffer>   m_depth_buf     = nullptr;
    std::shared_ptr<CommandPool>   m_cmd_pool      = nullptr;
    std::shared_ptr<RenderPass>    m_renderpass    = nullptr; // screen, loadOp=CLEAR
    std::shared_ptr<RenderPass>    m_renderpass_load = nullptr; // screen, loadOp=LOAD (resume)
    std::shared_ptr<FrameBuffers>  m_frame_buffers = nullptr;
    std::shared_ptr<PipelineCache> m_pipeline_cache= nullptr;

    mutable uint32_t m_current_buffer = 0;
    bool m_include_depth = false;

    // Per-frame batching state (see BeginFrameIfNeeded / BeginPass / Flush).
    bool m_frame_active       = false; // a frame is in progress (>=1 draw issued)
    bool m_swapchain_acquired = false; // a screen pass acquired an image this frame
    bool m_pass_open          = false; // a render pass is currently recording
    bool m_screen_cleared     = false; // the screen pass was already cleared this frame
    const vulkan::Framebuffer* m_cur_pass_fbo = nullptr; // null = screen
    VkExtent2D   m_cur_extent      = { 0, 0 };  // current pass render area (for viewport)
    VkRenderPass m_cur_render_pass = VK_NULL_HANDLE; // current pass's render pass
    uint32_t     m_cur_color_count = 1;              // color attachments of current pass

    // Diagnostic (TT_VK_DUMP): remember the last offscreen color target of the frame
    // so Flush can dump it + the swapchain image to PPM.
    int      m_frame_count          = 0;
    // Every distinct offscreen color target seen this frame (TT_VK_DUMP dumps all).
    struct DumpTarget { VkImage image; VkFormat fmt; uint32_t w; uint32_t h; };
    std::vector<DumpTarget> m_dump_targets;

    // On-demand graphics pipelines, built against the current render pass so they are
    // compatible with the pass they're used in. Keyed by (ShaderProgram*, target FBO*)
    // -- the same program needs a separate pipeline for the screen vs each offscreen
    // target (different attachment formats / depth presence).
    // Key: (program, target FBO, primitive topology). Topology matters because the
    // post-process / composite passes draw a tri-strip quad while meshes are tri-list.
    std::map<std::tuple<const void*, const void*, int>, std::shared_ptr<Pipeline>> m_prog_pipelines;

    // Framebuffer override (off-screen)
    std::shared_ptr<ur::Framebuffer> m_set_framebuffer = nullptr;

    // Bound texture/sampler slots (for next draw call)
    static constexpr size_t MAX_TEXTURE_SLOTS = 32;
    std::array<ur::TexturePtr, MAX_TEXTURE_SLOTS> m_bound_textures = {};
    std::array<std::shared_ptr<ur::TextureSampler>, MAX_TEXTURE_SLOTS> m_bound_samplers = {};

}; // Context

}
}
