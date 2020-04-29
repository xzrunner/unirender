#include "unirender/opengl/Context.h"
#include "unirender/opengl/Framebuffer.h"
#include "unirender/opengl/TypeConverter.h"
#include "unirender/opengl/VertexArray.h"
#include "unirender/opengl/RenderBuffer.h"
#include "unirender/opengl/VertexArray.h"
#include "unirender/opengl/TextureUnit.h"
#include "unirender/ClearState.h"
#include "unirender/DrawState.h"
#include "unirender/ShaderProgram.h"
#include "unirender/IndexBuffer.h"
#include "unirender/VertexArraySizes.h"
#include "unirender/Device.h"
#include "unirender/VertexBuffer.h"

#include <SM_Vector.h>

#include <assert.h>

namespace ur
{
namespace opengl
{

Context::Context(const ur::Device& device)
    : m_texture_units(device)
{
    Init();
}

void Context::Clear(const ClearState& clear_state)
{
    ApplyFramebuffer();

    ApplyScissorTest(clear_state.scissor_test);
    ApplyColorMask(clear_state.color_mask);
    ApplyDepthMask(clear_state.depth_mask);

    if (m_clear_color != clear_state.color)
    {
        const float r = clear_state.color.r / 255.0f;
        const float g = clear_state.color.g / 255.0f;
        const float b = clear_state.color.b / 255.0f;
        const float a = clear_state.color.a / 255.0f;
        glClearColor(r, g, b, a);

        m_clear_color = clear_state.color;
    }

    if (m_clear_depth != clear_state.depth)
    {
        glClearDepth(clear_state.depth);
        m_clear_depth = clear_state.depth;
    }

    if (m_clear_stencil != clear_state.stencil)
    {
        glClearStencil(clear_state.stencil);
        m_clear_stencil = clear_state.stencil;
    }

    glClear(TypeConverter::To(clear_state.buffers));
}

void Context::Draw(PrimitiveType prim_type, int offset, int count,
                   const DrawState& draw, const void* scene)
{
    assert(draw.program && draw.vertex_array);
    ApplyBeforeDraw(draw, scene);

    auto va = std::static_pointer_cast<VertexArray>(draw.vertex_array);

    auto ibuf = draw.vertex_array->GetIndexBuffer();
    if (ibuf)
    {
        glDrawRangeElements(TypeConverter::To(prim_type),
            0, va->GetMaxArrayIndex(), count,
            TypeConverter::To(ibuf->GetDataType()),
            (char *)0 + offset * VertexArraySizes::SizeOf(ibuf->GetDataType())
        );
    }
    else
    {
        glDrawArrays(TypeConverter::To(prim_type), offset, count);
    }
}

void Context::Draw(PrimitiveType prim_type, const DrawState& draw,
                   const void* scene)
{
    if (!draw.vertex_array) {
        return;
    }

    ApplyBeforeDraw(draw, scene);

    auto va = std::static_pointer_cast<VertexArray>(draw.vertex_array);
    auto ibuf = draw.vertex_array->GetIndexBuffer();

    if (ibuf)
    {
        if (draw.count == 0)
        {
            GLsizei count = ibuf->GetSizeInBytes() / VertexArraySizes::SizeOf(ibuf->GetDataType());
            glDrawRangeElements(TypeConverter::To(prim_type), 0, va->GetMaxArrayIndex(),
                count, TypeConverter::To(ibuf->GetDataType()), 0);
        }
        else
        {
            glDrawRangeElements(TypeConverter::To(prim_type), draw.offset, draw.offset + draw.count,
                draw.count, TypeConverter::To(ibuf->GetDataType()), 0);
        }
    }
    else
    {
        if (draw.count == 0) {
            glDrawArrays(TypeConverter::To(prim_type), 0, va->GetMaxArrayIndex() + 1);
        } else {
            glDrawArrays(TypeConverter::To(prim_type), draw.offset, draw.offset + draw.count);
        }
    }
}

void Context::SetViewport(int x, int y, int w, int h)
{
    Rectangle vp(x, y, w, h);
    if (m_viewport != vp)
    {
        glViewport(x, y, w, h);
        m_viewport = vp;
    }
}

void Context::GetViewport(int& x, int& y, int& w, int& h) const
{
    x = m_viewport.x;
    y = m_viewport.y;
    w = m_viewport.w;
    h = m_viewport.h;
}

void Context::SetTexture(size_t slot, const ur::TexturePtr& tex)
{
    auto unit = m_texture_units.GetUnit(slot);
    if (unit) {
        unit->SetTexture(tex);
    }
}

void Context::SetUnpackRowLength(int len)
{
    if (len != m_unpack_row_length) {
        m_unpack_row_length = len;
        glPixelStorei(GL_UNPACK_ROW_LENGTH, m_unpack_row_length);
    }
}

void Context::SetPackRowLength(int len)
{
    if (len != m_pack_row_length) {
        m_pack_row_length = len;
        glPixelStorei(GL_PACK_ROW_LENGTH, m_pack_row_length);
    }
}

bool Context::CheckRenderTargetStatus()
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch(status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		printf("%s", "Framebuffer incomplete: Attachment is NOT complete.\n");
		return false;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        printf("%s", "Framebuffer incomplete: No image is attached to FBO.\n");
		return false;
#if !defined(_WIN32) && !defined(__MACH__)
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        printf("%s", "Framebuffer incomplete: Attached images have different dimensions.\n");
		return false;
#endif
	case GL_FRAMEBUFFER_UNSUPPORTED:
        printf("%s", "Unsupported by FBO implementation.\n");
		return false;
	default:
        printf("%s", "Unknow error.\n");
		return false;
	}
}

void Context::Init()
{
    GLfloat col[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, col);
    m_clear_color.r = static_cast<uint8_t>(255 * col[0]);
    m_clear_color.g = static_cast<uint8_t>(255 * col[1]);
    m_clear_color.b = static_cast<uint8_t>(255 * col[2]);
    m_clear_color.a = static_cast<uint8_t>(255 * col[3]);

    GLdouble depth;
    glGetDoublev(GL_DEPTH_CLEAR_VALUE, &depth);
    m_clear_depth = depth;

    GLint stencil;
    glGetIntegerv(GL_STENCIL_CLEAR_VALUE, &stencil);
    m_clear_stencil = stencil;

    ForceApplyRenderState(m_render_state);
}

void Context::ForceApplyRenderState(const RenderState& rs)
{
    Enable(GL_PRIMITIVE_RESTART, rs.prim_restart.enabled);
    glPrimitiveRestartIndex(rs.prim_restart.index);

    Enable(GL_CULL_FACE, rs.facet_culling.enabled);
    glCullFace(TypeConverter::To(rs.facet_culling.face));
    glFrontFace(TypeConverter::To(rs.facet_culling.front_face_winding_order));

    Enable(GL_PROGRAM_POINT_SIZE, rs.prog_point_size == ProgramPointSize::Enabled);
    glPolygonMode(GL_FRONT_AND_BACK, TypeConverter::To(rs.rasterization_mode));

    Enable(GL_SCISSOR_TEST, rs.scissor_test.enabled);
    auto& rect = rs.scissor_test.rect;
    glScissor(rect.x, rect.y, rect.w, rect.h);

    Enable(GL_STENCIL_TEST, rs.stencil_test.enabled);
    ForceApplyRenderStateStencil(GL_FRONT, rs.stencil_test.front_face);
    ForceApplyRenderStateStencil(GL_BACK, rs.stencil_test.back_face);

    Enable(GL_DEPTH_TEST, rs.depth_test.enabled);
    glDepthFunc(TypeConverter::To(rs.depth_test.function));

    glDepthRange(rs.depth_range.d_near, rs.depth_range.d_far);

    Enable(GL_BLEND, rs.blending.enabled);
    glBlendFuncSeparate(
        TypeConverter::To(rs.blending.src_rgb),
        TypeConverter::To(rs.blending.dst_rgb),
        TypeConverter::To(rs.blending.src_alpha),
        TypeConverter::To(rs.blending.dst_alpha));
    glBlendEquationSeparate(
        TypeConverter::To(rs.blending.rgb_equation),
        TypeConverter::To(rs.blending.alpha_equation));
    glBlendColor(rs.blending.color.r, rs.blending.color.g, rs.blending.color.b, rs.blending.color.a);

    glDepthMask(rs.depth_mask);
    glColorMask(rs.color_mask.r, rs.color_mask.g, rs.color_mask.b, rs.color_mask.a);

    Enable(GL_ALPHA_TEST, rs.alpha_test.enabled);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, m_unpack_row_length);
    glPixelStorei(GL_PACK_ROW_LENGTH, m_pack_row_length);
}

void Context::ForceApplyRenderStateStencil(GLenum face, const StencilTestFace& test)
{
    glStencilOpSeparate(face,
        TypeConverter::To(test.stencil_fail_op),
        TypeConverter::To(test.depth_fail_stencil_pass_op),
        TypeConverter::To(test.depth_pass_stencil_pass_op));

    glStencilFuncSeparate(face,
        TypeConverter::To(test.function),
        test.reference_value,
        test.mask);
}

void Context::ApplyRenderState(const RenderState& rs)
{
    ApplyPrimitiveRestart(rs.prim_restart);
    ApplyFacetCulling(rs.facet_culling);
    ApplyProgramPointSize(rs.prog_point_size);
    ApplyRasterizationMode(rs.rasterization_mode);
    ApplyScissorTest(rs.scissor_test);
    ApplyStencilTest(rs.stencil_test);
    ApplyDepthTest(rs.depth_test);
    ApplyDepthRange(rs.depth_range);
    ApplyBlending(rs.blending);
    ApplyColorMask(rs.color_mask);
    ApplyDepthMask(rs.depth_mask);
    ApplyAlphaTest(rs.alpha_test);
}

void Context::ApplyFramebuffer()
{
    if (m_set_framebuffer != m_binded_framebuffer)
    {
        if (m_set_framebuffer) {
            m_set_framebuffer->Bind();
        } else {
            Framebuffer::UnBind();
        }

        m_binded_framebuffer = m_set_framebuffer;
    }

    if (m_set_framebuffer)
    {
        std::static_pointer_cast<opengl::Framebuffer>(m_set_framebuffer)->Clean();
        assert(CheckRenderTargetStatus());
    }
}

void Context::ApplyScissorTest(const ScissorTest& scissor)
{
    if (m_render_state.scissor_test.enabled != scissor.enabled)
    {
        Enable(GL_SCISSOR_TEST, scissor.enabled);
        m_render_state.scissor_test.enabled = scissor.enabled;
    }

    if (scissor.enabled)
    {
        if (m_render_state.scissor_test.rect != scissor.rect)
        {
            auto& r = scissor.rect;
            glScissor(r.x, r.y, r.w, r.h);
            m_render_state.scissor_test.rect = scissor.rect;
        }
    }
}

void Context::ApplyColorMask(const ColorMask& color_mask)
{
    if (m_render_state.color_mask != color_mask)
    {
        auto& m = color_mask;
        glColorMask(m.r, m.g, m.b, m.a);
        m_render_state.color_mask = color_mask;
    }
}

void Context::ApplyDepthMask(bool depth_mask)
{
    if (m_render_state.depth_mask != depth_mask)
    {
        glDepthMask(depth_mask);
        m_render_state.depth_mask = depth_mask;
    }
}

void Context::ApplyPrimitiveRestart(const PrimitiveRestart& prim_restart)
{
    if (m_render_state.prim_restart.enabled != prim_restart.enabled) {
        Enable(GL_PRIMITIVE_RESTART, prim_restart.enabled);
        m_render_state.prim_restart.enabled = prim_restart.enabled;
    }
    if (prim_restart.enabled &&
        m_render_state.prim_restart.index != prim_restart.index) {
        glPrimitiveRestartIndex(prim_restart.index);
        m_render_state.prim_restart.index = prim_restart.index;
    }
}

void Context::ApplyFacetCulling(const FacetCulling& culling)
{
    if (m_render_state.facet_culling.enabled != culling.enabled) {
        Enable(GL_CULL_FACE, culling.enabled);
        m_render_state.facet_culling.enabled = culling.enabled;
    }

    if (culling.enabled)
    {
        if (m_render_state.facet_culling.face != culling.face) {
            glCullFace(TypeConverter::To(culling.face));
            m_render_state.facet_culling.face = culling.face;
        }
        if (m_render_state.facet_culling.front_face_winding_order != culling.front_face_winding_order) {
            glFrontFace(TypeConverter::To(culling.front_face_winding_order));
            m_render_state.facet_culling.front_face_winding_order = culling.front_face_winding_order;
        }
    }
}

void Context::ApplyProgramPointSize(const ProgramPointSize& pt_sz)
{
    if (m_render_state.prog_point_size != pt_sz) {
        Enable(GL_PROGRAM_POINT_SIZE, pt_sz == ProgramPointSize::Enabled);
        m_render_state.prog_point_size = pt_sz;
    }
}

void Context::ApplyRasterizationMode(const RasterizationMode& mode)
{
    if (m_render_state.rasterization_mode != mode) {
        glPolygonMode(GL_FRONT_AND_BACK, TypeConverter::To(mode));
        m_render_state.rasterization_mode = mode;
    }
}

void Context::ApplyStencilTest(const StencilTest& stencil)
{
    if (m_render_state.stencil_test.enabled != stencil.enabled) {
        Enable(GL_STENCIL_TEST, stencil.enabled);
        m_render_state.stencil_test.enabled = stencil.enabled;
    }
    if (stencil.enabled) {
        ApplyStencil(GL_FRONT, m_render_state.stencil_test.front_face, stencil.front_face);
        ApplyStencil(GL_BACK, m_render_state.stencil_test.back_face, stencil.back_face);
    }
}

void Context::ApplyDepthTest(const DepthTest& depth)
{
    if (m_render_state.depth_test.enabled != depth.enabled) {
        Enable(GL_DEPTH_TEST, depth.enabled);
        m_render_state.depth_test.enabled = depth.enabled;
    }
    if (depth.enabled && m_render_state.depth_test.function != depth.function) {
        glDepthFunc(TypeConverter::To(depth.function));
        m_render_state.depth_test.function = depth.function;
    }
}

void Context::ApplyDepthRange(const DepthRange& range)
{
    assert(range.d_near >= 0 && range.d_near <= 1
        && range.d_far >= 0 && range.d_far <= 1);
    if (m_render_state.depth_range.d_near != range.d_near ||
        m_render_state.depth_range.d_far != range.d_far) {
        glDepthRange(range.d_near, range.d_far);
        m_render_state.depth_range.d_near = range.d_near;
        m_render_state.depth_range.d_far = range.d_far;
    }
}

void Context::ApplyBlending(const Blending& blending)
{
    if (m_render_state.blending.enabled != blending.enabled) {
        Enable(GL_BLEND, blending.enabled);
        m_render_state.blending.enabled = blending.enabled;
    }

    if (blending.enabled)
    {
        if (blending.separately)
        {
            if (m_render_state.blending.src_rgb != blending.src_rgb ||
                m_render_state.blending.dst_rgb != blending.dst_rgb ||
                m_render_state.blending.src_alpha != blending.src_alpha ||
                m_render_state.blending.dst_alpha != blending.dst_alpha)
            {
                glBlendFuncSeparate(
                    TypeConverter::To(blending.src_rgb),
                    TypeConverter::To(blending.dst_rgb),
                    TypeConverter::To(blending.src_alpha),
                    TypeConverter::To(blending.dst_alpha)
                );
                m_render_state.blending.src_rgb = blending.src_rgb;
                m_render_state.blending.dst_rgb = blending.dst_rgb;
                m_render_state.blending.src_alpha = blending.src_alpha;
                m_render_state.blending.dst_alpha = blending.dst_alpha;
            }
            if (m_render_state.blending.rgb_equation != blending.rgb_equation ||
                m_render_state.blending.alpha_equation != blending.alpha_equation)
            {
                glBlendEquationSeparate(
                    TypeConverter::To(blending.rgb_equation),
                    TypeConverter::To(blending.alpha_equation)
                );
                m_render_state.blending.rgb_equation = blending.rgb_equation;
                m_render_state.blending.alpha_equation = blending.alpha_equation;
            }
            if (m_render_state.blending.color != blending.color)
            {
                auto& c = blending.color;
                glBlendColor(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
                m_render_state.blending.color = blending.color;
            }
        }
        else
        {
            if (m_render_state.blending.src != blending.src ||
                m_render_state.blending.dst != blending.dst)
            {
                glBlendFunc(
                    TypeConverter::To(blending.src),
                    TypeConverter::To(blending.dst)
                );
                m_render_state.blending.src = blending.src;
                m_render_state.blending.dst = blending.dst;
            }
            if (m_render_state.blending.equation != blending.equation)
            {
                glBlendEquation(TypeConverter::To(blending.equation));
                m_render_state.blending.equation = blending.equation;
            }
        }
        m_render_state.blending.separately = blending.separately;
    }
}

void Context::ApplyAlphaTest(const AlphaTest& alpha)
{
    if (m_render_state.alpha_test.enabled != alpha.enabled) {
        Enable(GL_ALPHA_TEST, alpha.enabled);
        m_render_state.alpha_test.enabled = alpha.enabled;
    }
    if (alpha.enabled && (m_render_state.alpha_test.function != alpha.function
        || m_render_state.alpha_test.ref != alpha.ref)) {
        glAlphaFunc(TypeConverter::To(alpha.function), alpha.ref);
        m_render_state.alpha_test.function = alpha.function;
        m_render_state.alpha_test.ref      = alpha.ref;
    }
}

void Context::ApplyBeforeDraw(const DrawState& draw, const void* scene)
{
    ApplyRenderState(draw.render_state);
    ApplyVertexArray(draw.vertex_array);
    ApplyShaderProgram(draw, scene);

    m_texture_units.Clean();
    ApplyFramebuffer();
}

void Context::ApplyVertexArray(const std::shared_ptr<ur::VertexArray>& va)
{
    auto gl_va = std::static_pointer_cast<opengl::VertexArray>(va);
    gl_va->Bind();
    gl_va->Clean();
}

void Context::ApplyShaderProgram(const DrawState& draw, const void* scene)
{
    if (m_binded_program != draw.program)
    {
        if (draw.program) {
            draw.program->Bind();
        }
        m_binded_program = draw.program;
    }
    assert(!m_binded_program || m_binded_program && m_binded_program->CheckStatus());

    if (m_binded_program) {
        m_binded_program->Clean(*this, draw, scene);
    }
}

void Context::Enable(GLenum cap, bool enable)
{
    if (enable) {
        glEnable(cap);
    } else {
        glDisable(cap);
    }
}

void Context::ApplyStencil(GLenum face, StencilTestFace& curr,
                           const StencilTestFace& set)
{
    if (curr.stencil_fail_op != set.stencil_fail_op ||
        curr.depth_fail_stencil_pass_op != set.depth_fail_stencil_pass_op ||
        curr.depth_pass_stencil_pass_op != set.depth_pass_stencil_pass_op)
    {
        glStencilOpSeparate(face,
            TypeConverter::To(set.stencil_fail_op),
            TypeConverter::To(set.depth_fail_stencil_pass_op),
            TypeConverter::To(set.depth_pass_stencil_pass_op)
        );
        curr.stencil_fail_op = set.stencil_fail_op;
        curr.depth_fail_stencil_pass_op = set.depth_fail_stencil_pass_op;
        curr.depth_pass_stencil_pass_op = set.depth_pass_stencil_pass_op;
    }

    if (curr.function != set.function ||
        curr.reference_value != set.reference_value ||
        curr.mask != set.mask)
    {
        glStencilFuncSeparate(face, TypeConverter::To(set.function),
            set.reference_value, set.mask);
        curr.function = set.function;
        curr.reference_value = set.reference_value;
        curr.mask = set.mask;
    }
}

}
}