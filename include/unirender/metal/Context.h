#pragma once

#undef DrawState

#include "unirender/Context.h"
#include "unirender/ClearState.h"
#include "unirender/Rectangle.h"
#include "unirender/typedef.h"

#include <array>
#include <memory>

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
    void* m_cmd_buffer      = nullptr;  // id<MTLCommandBuffer>
    void* m_render_encoder  = nullptr;  // id<MTLRenderCommandEncoder>
    void* m_drawable        = nullptr;  // id<CAMetalDrawable>
    void* m_depth_texture   = nullptr;  // id<MTLTexture>
    void* m_depth_stencil_state = nullptr; // id<MTLDepthStencilState>

    bool m_frame_active = false;

    // Bound texture / sampler slots
    static constexpr size_t MAX_SLOTS = 32;
    std::array<ur::TexturePtr, MAX_SLOTS> m_bound_textures = {};
    std::array<std::shared_ptr<ur::TextureSampler>, MAX_SLOTS> m_bound_samplers = {};

}; // Context

}
}
