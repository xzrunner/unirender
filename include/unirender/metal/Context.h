#pragma once

#undef DrawState

#include "unirender/Context.h"
#include "unirender/ClearState.h"
#include "unirender/Rectangle.h"
#include "unirender/typedef.h"

#include <array>
#include <memory>
#include <unordered_map>
#include <cstdint>

#undef DrawState

namespace ur
{

class Device;

namespace metal
{

class Device;

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
        int num_groups_y, int num_groups_z) override;

    virtual void SetViewport(int x, int y, int w, int h) override;
    virtual void GetViewport(int& x, int& y, int& w, int& h) const override;

    virtual void SetTexture(size_t slot, const ur::TexturePtr& tex) override;
    virtual void SetTextureSampler(size_t slot, const std::shared_ptr<ur::TextureSampler>& sampler) override;
    virtual void SetImage(size_t slot, const ur::TexturePtr& tex, AccessType access) override;

    virtual void SetFramebuffer(const std::shared_ptr<ur::Framebuffer>& fb) override;
    virtual std::shared_ptr<ur::Framebuffer> GetFramebuffer() const override;

    virtual void SetUnpackRowLength(int len) override {}
    virtual void SetPackRowLength(int len) override {}

    virtual bool CheckRenderTargetStatus() override { return true; }

    virtual void Flush() override;

    virtual std::shared_ptr<ur::Pipeline> CreatePipeline(bool include_depth, bool include_vi,
        const ur::PipelineLayout& layout, const ur::VertexBuffer& vb,
        const ur::ShaderProgram& prog) const override;

    virtual void SetMemoryBarrier(const std::vector<BarrierType>& types) override;

    int  GetWidth()  const { return m_width; }
    int  GetHeight() const { return m_height; }

private:
    void Init(void* hwnd, uint32_t width, uint32_t height);
    void BeginFrame();
    void EndFrame();
    // Start a render pass to the given color target(s). Ends the current encoder
    // first. The screen path binds one target (the drawable); an FBO can bind
    // several (MRT, e.g. the deferred GBuffer). depth_tex, when non-null, is
    // attached as the depth/stencil buffer; clear uses m_clear_state, otherwise
    // the existing content is loaded.
    void BeginColorPass(void* const* color_texs, size_t color_count,
                        void* depth_tex, bool clear);
    void EndEncoder();
    // Lazily create the per-frame command buffer that holds BOTH the offscreen
    // (atlas) passes and the screen passes -- created on whichever happens first.
    void EnsureCmdBuffer();

    // Create the screen depth/stencil texture. On Apple-family (TBDR) GPUs it is
    // MTLStorageModeMemoryless (lives only in tile memory, zero DRAM) since it is
    // never sampled and never stored; falls back to Private on Intel Macs. Returns a
    // __bridge_retained id<MTLTexture> the caller owns. Used by Init and Resize.
    void* MakeDepthTexture(uint32_t width, uint32_t height) const;

    // Build or retrieve a MTLRenderPipelineState for the given DrawState
    void* GetOrCreatePipelineState(const DrawState& draw);

    const metal::Device& m_device;

    uint32_t m_width  = 0;
    uint32_t m_height = 0;

    Rectangle m_viewport;

    // Clear state — applied at render-pass begin
    ClearState m_clear_state;

    std::shared_ptr<ur::Framebuffer> m_set_framebuffer = nullptr;

    // Metal objects (opaque pointers)
    void* m_mtl_layer       = nullptr;  // CAMetalLayer*
    void* m_frame_sem       = nullptr;  // dispatch_semaphore_t (1 frame in flight)
    void* m_cmd_buffer      = nullptr;  // id<MTLCommandBuffer>
    void* m_render_encoder  = nullptr;  // id<MTLRenderCommandEncoder>
    void* m_drawable        = nullptr;  // id<CAMetalDrawable>
    void* m_depth_texture   = nullptr;  // id<MTLTexture> (screen depth/stencil)

    // Current color render targets: the screen path binds one (the drawable); an
    // FBO can bind several (MRT, e.g. the deferred GBuffer). The pipeline's color
    // attachment formats must match these one-for-one, in order.
    static constexpr size_t MAX_COLOR_ATTACH = 8;
    std::array<void*, MAX_COLOR_ATTACH> m_cur_color_targets = {}; // id<MTLTexture>
    size_t m_cur_color_count = 0;
    // Depth/stencil target of the active pass (the screen depth texture, or an
    // FBO's depth attachment); null when the pass has no depth. The pipeline's
    // depth format and the per-draw depth-stencil state must match this.
    void* m_cur_depth_target = nullptr;  // id<MTLTexture>
    bool  m_cur_has_depth    = false;
    // True while the active pass targets an offscreen FBO (not the screen drawable);
    // such passes use the clip-space-y-flipped vertex function (GL vs Metal origin).
    bool  m_cur_is_fbo       = false;

    bool m_frame_active = false;
    // True when m_cmd_buffer is a transient buffer created for offscreen
    // (render-to-texture) work that happens outside a screen frame, e.g. the
    // dtex glyph atlas. It is committed when the FBO is unbound.
    bool m_offscreen_cmd = false;

    // Bound texture / sampler slots
    static constexpr size_t MAX_SLOTS = 32;
    std::array<ur::TexturePtr, MAX_SLOTS> m_bound_textures = {};
    std::array<std::shared_ptr<ur::TextureSampler>, MAX_SLOTS> m_bound_samplers = {};

    // Pipeline-state caches. Building an MTLRenderPipelineState (and, less so, an
    // MTLDepthStencilState) is expensive -- it compiles/links the shader functions --
    // so we build each unique configuration once and reuse it, instead of rebuilding
    // one per draw call (the GL "switching programs is cheap" assumption). Each entry
    // is a __bridge_retained Metal object OWNED by the cache and released in the
    // destructor; GetOrCreatePipelineState/Draw hand out borrowed (non-owning)
    // pointers. Keyed by a hash of everything that affects the state object.
    std::unordered_map<uint64_t, void*> m_pso_cache; // id<MTLRenderPipelineState>
    std::unordered_map<uint64_t, void*> m_dss_cache; // id<MTLDepthStencilState>

}; // Context

}
}
