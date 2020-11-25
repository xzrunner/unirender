#pragma once

#include "unirender/Context.h"
#include "unirender/RenderState.h"
#include "unirender/PrimitiveType.h"
#include "unirender/opengl/opengl.h"
#include "unirender/opengl/TextureUnits.h"

#include <memory>

namespace ur
{

struct ClearState;
struct DrawState;
class ShaderProgram;
class VertexArray;

namespace opengl
{

class Context : public ur::Context
{
public:
    Context(const ur::Device& device);

    virtual void Resize(uint32_t width, uint32_t height) override {}

    virtual void Clear(const ClearState& clear_state) override;
    virtual void Draw(PrimitiveType prim_type, int offset, int count,
        const DrawState& draw, const void* scene) override;
    virtual void Draw(PrimitiveType prim_type, const DrawState& draw,
        const void* scene) override;
    virtual void Compute(const DrawState& draw, int thread_group_count) override;

    virtual void SetViewport(int x, int y, int w, int h) override;
    virtual void GetViewport(int& x, int& y, int& w, int& h) const override;

    virtual void SetTexture(size_t slot, const ur::TexturePtr& tex) override;
    virtual void SetTextureSampler(size_t slot, const std::shared_ptr<ur::TextureSampler>& sampler) override;
    virtual void SetFramebuffer(const std::shared_ptr<ur::Framebuffer>& fb) override {
        m_set_framebuffer = fb;
    }
    virtual std::shared_ptr<ur::Framebuffer> GetFramebuffer() const override {
        return m_set_framebuffer;
    }

    virtual void SetUnpackRowLength(int len) override;
    virtual void SetPackRowLength(int len) override;

    virtual bool CheckRenderTargetStatus() override;

    virtual void Flush() override;

    virtual std::shared_ptr<Pipeline> CreatePipeline(bool include_depth, bool include_vi, const ur::PipelineLayout& layout,
        const ur::VertexBuffer& vb, const ur::ShaderProgram& prog) const override { return nullptr; }

private:
    void Init();

    void ForceApplyRenderState(const RenderState& rs);
    void ForceApplyRenderStateStencil(GLenum face, const StencilTestFace& test);

    void ApplyRenderState(const RenderState& rs);
    void ApplyFramebuffer();

    void ApplyScissorTest(const ScissorTest& scissor);
    void ApplyColorMask(const ColorMask& color_mask);
    void ApplyDepthMask(bool depth_mask);

    void ApplyPrimitiveRestart(const PrimitiveRestart& prim_restart);
    void ApplyFacetCulling(const FacetCulling& culling);
    void ApplyProgramPointSize(const ProgramPointSize& pt_sz);
    void ApplyRasterizationMode(const RasterizationMode& mode);
    void ApplyStencilTest(const StencilTest& stencil);
    void ApplyDepthTest(const DepthTest& depth);
    void ApplyDepthRange(const DepthRange& range);
    void ApplyBlending(const Blending& blending);
    void ApplyAlphaTest(const AlphaTest& alpha);
    void ApplyTessParams(const TessPatchParams& tess_params);

    void ApplyBeforeDraw(const DrawState& draw, const void* scene);

    void ApplyVertexArray(const std::shared_ptr<ur::VertexArray>& va);
    void ApplyShaderProgram(const DrawState& draw, const void* scene);

    void Enable(GLenum cap, bool enable);

    static void ApplyStencil(GLenum face, StencilTestFace& curr,
        const StencilTestFace& set);

private:
    const ur::Device& m_dev;

    Color  m_clear_color   = Color(0, 0, 0, 0);
    double m_clear_depth   = 0;
    int    m_clear_stencil = 0;
    Rectangle m_viewport;

    int m_unpack_row_length = 0;
    int m_pack_row_length = 0;

    RenderState m_render_state;

    std::shared_ptr<ShaderProgram> m_binded_program = nullptr;

    std::shared_ptr<Framebuffer> m_binded_framebuffer = nullptr;
    std::shared_ptr<Framebuffer> m_set_framebuffer    = nullptr;

    TextureUnits m_texture_units;

}; // Context

}
}