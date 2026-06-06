#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <TargetConditionals.h>
#if TARGET_OS_OSX
#import <AppKit/AppKit.h>      // NSView
#endif

#include "unirender/metal/Context.h"
#include "unirender/metal/Device.h"
#include "unirender/metal/VertexBuffer.h"
#include "unirender/metal/IndexBuffer.h"
#include "unirender/metal/Texture.h"
#include "unirender/metal/TextureSampler.h"
#include "unirender/metal/ShaderProgram.h"
#include "unirender/metal/Framebuffer.h"
#include "unirender/metal/RenderBuffer.h"
#include "unirender/metal/UniformBuffer.h"
#include "unirender/DrawState.h"
#include "unirender/ClearState.h"
#include "unirender/VertexInputAttribute.h"
#include "unirender/VertexArray.h"
#include "unirender/IndexBuffer.h"
#include "unirender/VertexBuffer.h"
#include "unirender/ShaderProgram.h"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <vector>

namespace
{

MTLPrimitiveType ToMTL(ur::PrimitiveType pt)
{
    switch (pt) {
    case ur::PrimitiveType::Points:        return MTLPrimitiveTypePoint;
    case ur::PrimitiveType::Lines:         return MTLPrimitiveTypeLine;
    case ur::PrimitiveType::LineStrip:     return MTLPrimitiveTypeLineStrip;
    case ur::PrimitiveType::Triangles:     return MTLPrimitiveTypeTriangle;
    case ur::PrimitiveType::TriangleStrip: return MTLPrimitiveTypeTriangleStrip;
    default:                               return MTLPrimitiveTypeTriangle;
    }
}

MTLIndexType ToMTLIndexType(ur::IndexBufferDataType dt)
{
    return (dt == ur::IndexBufferDataType::UnsignedInt)
        ? MTLIndexTypeUInt32
        : MTLIndexTypeUInt16;
}

MTLCompareFunction ToMTLCompare(ur::DepthTestFunc f)
{
    switch (f) {
    case ur::DepthTestFunc::Never:              return MTLCompareFunctionNever;
    case ur::DepthTestFunc::Less:               return MTLCompareFunctionLess;
    case ur::DepthTestFunc::Equal:              return MTLCompareFunctionEqual;
    case ur::DepthTestFunc::LessThanOrEqual:    return MTLCompareFunctionLessEqual;
    case ur::DepthTestFunc::Greater:            return MTLCompareFunctionGreater;
    case ur::DepthTestFunc::NotEqual:           return MTLCompareFunctionNotEqual;
    case ur::DepthTestFunc::GreaterThanOrEqual: return MTLCompareFunctionGreaterEqual;
    case ur::DepthTestFunc::Always:             return MTLCompareFunctionAlways;
    default:                                    return MTLCompareFunctionLess;
    }
}

MTLBlendFactor ToMTLBlend(ur::BlendingFactor f)
{
    switch (f) {
    case ur::BlendingFactor::Zero:                  return MTLBlendFactorZero;
    case ur::BlendingFactor::One:                   return MTLBlendFactorOne;
    case ur::BlendingFactor::SrcColor:              return MTLBlendFactorSourceColor;
    case ur::BlendingFactor::OneMinusSrcColor:      return MTLBlendFactorOneMinusSourceColor;
    case ur::BlendingFactor::DstColor:              return MTLBlendFactorDestinationColor;
    case ur::BlendingFactor::OneMinusDstColor:      return MTLBlendFactorOneMinusDestinationColor;
    case ur::BlendingFactor::SrcAlpha:              return MTLBlendFactorSourceAlpha;
    case ur::BlendingFactor::OneMinusSrcAlpha:      return MTLBlendFactorOneMinusSourceAlpha;
    case ur::BlendingFactor::DstAlpha:              return MTLBlendFactorDestinationAlpha;
    case ur::BlendingFactor::OneMinusDstAlpha:      return MTLBlendFactorOneMinusDestinationAlpha;
    case ur::BlendingFactor::ConstantColor:         return MTLBlendFactorBlendColor;
    case ur::BlendingFactor::OneMinusConstantColor: return MTLBlendFactorOneMinusBlendColor;
    case ur::BlendingFactor::ConstantAlpha:         return MTLBlendFactorBlendAlpha;
    case ur::BlendingFactor::OneMinusConstantAlpha: return MTLBlendFactorOneMinusBlendAlpha;
    case ur::BlendingFactor::SrcAlphaSaturate:      return MTLBlendFactorSourceAlphaSaturated;
    default:                                        return MTLBlendFactorOne;
    }
}

MTLBlendOperation ToMTLBlendOp(ur::BlendEquation e)
{
    switch (e) {
    case ur::BlendEquation::Add:             return MTLBlendOperationAdd;
    case ur::BlendEquation::Minimum:         return MTLBlendOperationMin;
    case ur::BlendEquation::Maximum:         return MTLBlendOperationMax;
    case ur::BlendEquation::Subtract:        return MTLBlendOperationSubtract;
    case ur::BlendEquation::ReverseSubtract: return MTLBlendOperationReverseSubtract;
    default:                                 return MTLBlendOperationAdd;
    }
}

}

namespace ur
{
namespace metal
{

// ===========================================================================
// Construction / destruction
// ===========================================================================

Context::Context(const ur::Device& device, void* hwnd,
                 uint32_t width, uint32_t height)
    : m_device(static_cast<const metal::Device&>(device))
{
    Init(hwnd, width, height);
}

Context::~Context()
{
    if (m_depth_texture)       { CFRelease(m_depth_texture); }
    if (m_frame_sem)           { CFRelease(m_frame_sem); m_frame_sem = nullptr; }
    // m_mtl_layer is bridged from the view, not owned
}

// ===========================================================================
// Init
// ===========================================================================

void Context::Init(void* hwnd, uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    // One frame in flight: shared per-frame resources (uniform buffers) have a
    // single copy, so the CPU must not record the next frame until the GPU has
    // finished the current one. See EnsureCmdBuffer / EndFrame.
    m_frame_sem = (__bridge_retained void*)dispatch_semaphore_create(1);

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device.GetMTLDevice();

    // --- Attach CAMetalLayer to native view ---
    if (hwnd)
    {
        CAMetalLayer* layer = [CAMetalLayer layer];
        layer.device          = device;
        layer.pixelFormat     = MTLPixelFormatBGRA8Unorm;
        layer.framebufferOnly = YES;
        layer.contentsScale   = 1.0; // 1 drawable pixel per point (match the editor's point coords)
        layer.drawableSize    = CGSizeMake(width, height);

#if TARGET_OS_OSX
        NSView* view = (__bridge NSView*)hwnd;
        [view setWantsLayer:YES];
        [view setLayer:layer];
#else
        // iOS: usually done via layerClass override
#endif
        m_mtl_layer = (__bridge_retained void*)layer;
    }

    // --- Depth/Stencil texture ---
    {
        MTLTextureDescriptor* dd =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
                                                              width:width height:height mipmapped:NO];
        dd.storageMode = MTLStorageModePrivate;
        dd.usage       = MTLTextureUsageRenderTarget;
        id<MTLTexture> dt = [device newTextureWithDescriptor:dd];
        m_depth_texture = (__bridge_retained void*)dt;
    }

    // The depth-stencil STATE is no longer global: each Draw() builds its own from
    // the draw's render_state (depth_test / depth_func / depth_mask), so 2D draws
    // get Always + no-write while the 3D GBuffer pass gets Less + write.

    // Default clear state
    m_clear_state.color = Color(0, 0, 0, 255);
    m_clear_state.depth = 1.0;
    m_clear_state.stencil = 0;
}

// ===========================================================================
// Resize
// ===========================================================================

void Context::Resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) return;
    m_width  = width;
    m_height = height;

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device.GetMTLDevice();

    if (m_mtl_layer) {
        CAMetalLayer* layer = (__bridge CAMetalLayer*)m_mtl_layer;
        layer.drawableSize = CGSizeMake(width, height);
    }

    // Recreate depth texture
    if (m_depth_texture) { CFRelease(m_depth_texture); }
    MTLTextureDescriptor* dd =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
                                                          width:width height:height mipmapped:NO];
    dd.storageMode = MTLStorageModePrivate;
    dd.usage       = MTLTextureUsageRenderTarget;
    id<MTLTexture> dt = [device newTextureWithDescriptor:dd];
    m_depth_texture = (__bridge_retained void*)dt;
}

// ===========================================================================
// Frame management
// ===========================================================================

void Context::EndEncoder()
{
    if (m_render_encoder) {
        id<MTLRenderCommandEncoder> encoder =
            (__bridge_transfer id<MTLRenderCommandEncoder>)m_render_encoder;
        [encoder endEncoding];
        m_render_encoder = nullptr;
    }
}

void Context::BeginColorPass(void* const* color_texs, size_t color_count,
                             void* depth_tex, bool clear)
{
    EndEncoder();

    id<MTLCommandBuffer> cmdBuf = (__bridge id<MTLCommandBuffer>)m_cmd_buffer;
    if (!cmdBuf || color_count == 0 || !color_texs[0]) { return; }
    if (color_count > MAX_COLOR_ATTACH) { color_count = MAX_COLOR_ATTACH; }

    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];
    for (size_t i = 0; i < color_count; ++i) {
        id<MTLTexture> colorTex = (__bridge id<MTLTexture>)color_texs[i];
        if (!colorTex) { continue; }
        rpd.colorAttachments[i].texture     = colorTex;
        rpd.colorAttachments[i].loadAction  = clear ? MTLLoadActionClear : MTLLoadActionLoad;
        rpd.colorAttachments[i].storeAction = MTLStoreActionStore;
        if (clear) {
            rpd.colorAttachments[i].clearColor =
                MTLClearColorMake(m_clear_state.color.r / 255.0,
                                  m_clear_state.color.g / 255.0,
                                  m_clear_state.color.b / 255.0,
                                  m_clear_state.color.a / 255.0);
        }
    }

    id<MTLTexture> depthTex = (__bridge id<MTLTexture>)depth_tex;
    if (depthTex) {
        rpd.depthAttachment.texture     = depthTex;
        rpd.depthAttachment.loadAction  = clear ? MTLLoadActionClear : MTLLoadActionLoad;
        rpd.depthAttachment.storeAction = MTLStoreActionDontCare;
        rpd.depthAttachment.clearDepth  = m_clear_state.depth;
        // Only attach stencil when the depth texture actually carries it (the
        // screen uses Depth32Float_Stencil8; an FBO depth RBO is plain depth).
        if (depthTex.pixelFormat == MTLPixelFormatDepth32Float_Stencil8 ||
            depthTex.pixelFormat == MTLPixelFormatDepth24Unorm_Stencil8) {
            rpd.stencilAttachment.texture      = depthTex;
            rpd.stencilAttachment.loadAction   = clear ? MTLLoadActionClear : MTLLoadActionLoad;
            rpd.stencilAttachment.storeAction  = MTLStoreActionDontCare;
            rpd.stencilAttachment.clearStencil = m_clear_state.stencil;
        }
    }

    id<MTLRenderCommandEncoder> encoder = [cmdBuf renderCommandEncoderWithDescriptor:rpd];
    if (!encoder) {
        std::cerr << "[Metal] BeginColorPass: renderCommandEncoderWithDescriptor returned nil"
                  << " n=" << color_count << " clear=" << clear << "\n";
        return;
    }
    m_render_encoder = (__bridge_retained void*)encoder;

    id<MTLTexture> firstColor = (__bridge id<MTLTexture>)color_texs[0];
    MTLViewport vp;
    vp.originX = m_viewport.x;
    vp.originY = m_viewport.y;
    vp.width   = (m_viewport.w > 0) ? m_viewport.w : (double)firstColor.width;
    vp.height  = (m_viewport.h > 0) ? m_viewport.h : (double)firstColor.height;
    vp.znear   = 0.0;
    vp.zfar    = 1.0;
    [encoder setViewport:vp];

    // Record the active targets so the pipeline can format-match them and the
    // per-draw depth-stencil state can be applied in Draw(). The depth-stencil
    // state is no longer set here -- each draw supplies its own from render_state.
    m_cur_color_count = color_count;
    for (size_t i = 0; i < MAX_COLOR_ATTACH; ++i) {
        m_cur_color_targets[i] = (i < color_count) ? color_texs[i] : nullptr;
    }
    m_cur_depth_target = depth_tex;
    m_cur_has_depth    = (depth_tex != nullptr);
    // m_cur_is_fbo is set by the caller (screen drawable vs offscreen FBO) before
    // this call; FBO passes render with a clip-space y flip (Metal vs GL origin).
}

void Context::EnsureCmdBuffer()
{
    if (!m_cmd_buffer) {
        // Throttle the CPU to one frame in flight. Shaders' uniform blocks (and the
        // sprite renderer's UBO) are single shared MTLBuffers the CPU memcpys into
        // every frame; without this the CPU races ahead and overwrites them while
        // the GPU is still reading the previous frame -> intermittent flicker of the
        // 2D text/nodes even when idle. The slot is returned in EndFrame's completion
        // handler once the GPU is done. (Dynamic vertex/index buffers are already
        // orphaned per write, so only the uniform buffers need this.)
        if (m_frame_sem) {
            dispatch_semaphore_wait((__bridge dispatch_semaphore_t)m_frame_sem,
                                    DISPATCH_TIME_FOREVER);
        }
        id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_device.GetCommandQueue();
        id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
        m_cmd_buffer = (__bridge_retained void*)cmdBuf;
    }
}

void Context::BeginFrame()
{
    if (m_frame_active) return;
    if (!m_mtl_layer) return;

    CAMetalLayer* layer = (__bridge CAMetalLayer*)m_mtl_layer;
    id<CAMetalDrawable> drawable = [layer nextDrawable];
    if (!drawable) return;
    m_drawable = (__bridge_retained void*)drawable;

    // Reuse the command buffer if offscreen (atlas) passes already started it this
    // frame, so those passes execute before the screen passes that sample them.
    EnsureCmdBuffer();

    m_frame_active = true;
    m_cur_is_fbo = false;
    void* color = (__bridge void*)drawable.texture;
    BeginColorPass(&color, 1, m_depth_texture, /*clear*/true);
}

void Context::EndFrame()
{
    EndEncoder();
    m_cur_color_count  = 0;
    m_cur_depth_target = nullptr;
    m_cur_has_depth    = false;

    // Commit whatever was recorded this frame (atlas passes and/or screen passes).
    if (m_cmd_buffer) {
        id<MTLCommandBuffer> cmdBuf = (__bridge_transfer id<MTLCommandBuffer>)m_cmd_buffer;
        if (m_drawable) {
            id<CAMetalDrawable> drawable = (__bridge_transfer id<CAMetalDrawable>)m_drawable;
            [cmdBuf presentDrawable:drawable];
            m_drawable = nullptr;
        }
        // Release the frame-in-flight slot only once the GPU has finished reading
        // this frame's shared resources (uniform buffers etc.), so the CPU can't
        // overwrite them while in use. Pairs with the wait in EnsureCmdBuffer.
        if (m_frame_sem) {
            dispatch_semaphore_t sem = (__bridge dispatch_semaphore_t)m_frame_sem;
            [cmdBuf addCompletedHandler:^(id<MTLCommandBuffer>) {
                dispatch_semaphore_signal(sem);
            }];
        }
        [cmdBuf commit];
        m_cmd_buffer = nullptr;
    }

    m_frame_active = false;
}

// ===========================================================================
// Clear
// ===========================================================================

void Context::Clear(const ClearState& clear_state)
{
    m_clear_state = clear_state;

    // GL clears the bound framebuffer immediately; on Metal a clear happens via the
    // render pass loadAction at pass start. If no pass is active yet, the values are
    // applied at the next BeginColorPass (the screen BeginFrame, or an FBO bind) --
    // the original behaviour. But if a pass IS already running, this is the
    // rendergraph clearing an FBO it just bound, right before drawing into it (e.g.
    // the deferred GBuffer's color + depth). Without honoring it the attachment keeps
    // its stale/garbage contents -- an uncleared GBuffer depth rejects the mesh under
    // depth-test, so the box never appears. Re-begin the pass on the same targets
    // with a clear load action so the clear actually happens. Restrict this to FBO
    // passes: the screen drawable is cleared once at BeginFrame, and restarting it
    // mid-frame from a stray Clear() would wipe everything already drawn this frame.
    if (!m_render_encoder || m_cur_color_count == 0 || !m_cur_is_fbo) {
        return;
    }

    // A scissored clear (dtex atlas block reuse) must touch only a sub-rect; a
    // loadAction clear wipes the whole attachment, so don't -- leave it intact
    // (same as the old no-op) rather than destroy the other packed glyphs.
    if (clear_state.scissor_test.enabled) {
        return;
    }

    std::array<void*, MAX_COLOR_ATTACH> colors = m_cur_color_targets;
    const size_t n = m_cur_color_count;
    void* depth = m_cur_depth_target;
    // m_cur_is_fbo stays as the pass we are restarting set it.
    BeginColorPass(colors.data(), n, depth, /*clear*/true);
}

// ===========================================================================
// Draw
// ===========================================================================

void Context::Draw(PrimitiveType prim_type, int offset, int count,
                   const DrawState& draw, const void* scene)
{
    if (count <= 0) return;
    if (!m_render_encoder) BeginFrame(); // use the active pass (FBO atlas or screen); else start the screen frame
    if (!m_render_encoder) return;

    id<MTLRenderCommandEncoder> encoder =
        (__bridge id<MTLRenderCommandEncoder>)m_render_encoder;

    // --- Bind pipeline state ---
    void* pso = GetOrCreatePipelineState(draw);
    if (pso) {
        [encoder setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)pso];
    }

    // --- Depth-stencil + face culling from the draw's render state. Only when
    //     the active pass has a depth attachment (the screen pass, or an FBO with
    //     a depth buffer like the deferred GBuffer). 2D draws disable the depth
    //     test, giving Always + no-write -- the old fixed behaviour. ---
    if (m_cur_has_depth) {
        const auto& rs = draw.render_state;
        MTLDepthStencilDescriptor* dsd = [[MTLDepthStencilDescriptor alloc] init];
        dsd.depthCompareFunction = rs.depth_test.enabled
            ? ToMTLCompare(rs.depth_test.function) : MTLCompareFunctionAlways;
        dsd.depthWriteEnabled = (rs.depth_test.enabled && rs.depth_mask) ? YES : NO;
        id<MTLDevice> dev = (__bridge id<MTLDevice>)m_device.GetMTLDevice();
        id<MTLDepthStencilState> dss = [dev newDepthStencilStateWithDescriptor:dsd];
        [encoder setDepthStencilState:dss];
    }
    {
        const auto& fc = draw.render_state.facet_culling;
        if (fc.enabled) {
            [encoder setCullMode:(fc.face == ur::CullFace::Front ? MTLCullModeFront
                                                                 : MTLCullModeBack)];
            [encoder setFrontFacingWinding:
                (fc.front_face_winding_order == ur::WindingOrder::Clockwise
                    ? MTLWindingClockwise : MTLWindingCounterClockwise)];
        } else {
            [encoder setCullMode:MTLCullModeNone];
        }
    }

    // --- Bind vertex buffer + reflected uniform blocks. The shader picks the
    //     MSL buffer indices so vertex (stage_in) data and UBOs never collide. ---
    auto mtl_prog = draw.program
        ? std::static_pointer_cast<metal::ShaderProgram>(draw.program) : nullptr;
    const int vtx_index = mtl_prog ? mtl_prog->GetVertexBufferIndex() : 0;

    // Run the shader's uniform updaters -- writes matrices etc. into the UBO
    // backing buffers (same as the GL backend's program->Clean() before a draw).
    if (draw.program) {
        draw.program->Clean(*this, draw, scene);
    }

    if (draw.vertex_array) {
        auto vb = draw.vertex_array->GetVertexBuffer();
        if (vb) {
            auto mtl_vb = std::static_pointer_cast<metal::VertexBuffer>(vb);
            if (mtl_vb->GetMTLBuffer()) {
                id<MTLBuffer> buf = (__bridge id<MTLBuffer>)mtl_vb->GetMTLBuffer();
                [encoder setVertexBuffer:buf offset:0 atIndex:vtx_index];
            }
        }
    }

    if (mtl_prog) {
        for (auto& ubo : mtl_prog->GetUBOs()) {
            if (!ubo.mtl_buffer) { continue; }
            id<MTLBuffer> b = (__bridge id<MTLBuffer>)ubo.mtl_buffer;
            if (ubo.stage == ShaderType::VertexShader) {
                [encoder setVertexBuffer:b offset:0 atIndex:ubo.buffer_index];
            } else {
                [encoder setFragmentBuffer:b offset:0 atIndex:ubo.buffer_index];
            }
        }
    }

    // --- Bind textures & samplers. The MSL index a sampled image binds to comes
    //     from reflection (HLSL register(tN) -> SPIR-V binding -> MSL [[texture(M)]])
    //     and is not necessarily the engine's SetTexture() slot, so map each
    //     engine-bound texture (in ascending slot order) onto the shader's reflected
    //     tex/sampler MSL indices (ascending). Metal also needs a sampler bound even
    //     when the engine passes a null one (SetTextureSampler(slot, nullptr)) --
    //     an unbound MSL sampler reads 0. ---
    std::vector<int> shader_tex_idx, shader_smp_idx;
    if (mtl_prog) {
        for (auto& kv : mtl_prog->GetTexSlots())     { shader_tex_idx.push_back(kv.second); }
        for (auto& kv : mtl_prog->GetSamplerSlots()) { shader_smp_idx.push_back(kv.second); }
        std::sort(shader_tex_idx.begin(), shader_tex_idx.end());
        std::sort(shader_smp_idx.begin(), shader_smp_idx.end());
    }
    size_t tex_ord = 0;
    for (size_t i = 0; i < MAX_SLOTS; ++i) {
        if (!m_bound_textures[i]) { continue; }
        auto mtl_tex = std::static_pointer_cast<metal::Texture>(m_bound_textures[i]);
        if (!mtl_tex->GetMTLTexture()) { continue; }

        NSUInteger tex_index = (tex_ord < shader_tex_idx.size())
            ? (NSUInteger)shader_tex_idx[tex_ord] : (NSUInteger)i;
        NSUInteger smp_index = (tex_ord < shader_smp_idx.size())
            ? (NSUInteger)shader_smp_idx[tex_ord] : tex_index;
        ++tex_ord;

        [encoder setFragmentTexture:(__bridge id<MTLTexture>)mtl_tex->GetMTLTexture() atIndex:tex_index];

        id<MTLSamplerState> samp = nil;
        if (m_bound_samplers[i]) {
            auto mtl_s = std::static_pointer_cast<metal::TextureSampler>(m_bound_samplers[i]);
            if (mtl_s->GetMTLSamplerState()) {
                samp = (__bridge id<MTLSamplerState>)mtl_s->GetMTLSamplerState();
            }
        }
        if (!samp) {
            static id<MTLSamplerState> s_default = nil;
            if (!s_default) {
                MTLSamplerDescriptor* sd = [[MTLSamplerDescriptor alloc] init];
                sd.minFilter    = MTLSamplerMinMagFilterLinear;
                sd.magFilter    = MTLSamplerMinMagFilterLinear;
                sd.sAddressMode = MTLSamplerAddressModeClampToEdge;
                sd.tAddressMode = MTLSamplerAddressModeClampToEdge;
                id<MTLDevice> dev = (__bridge id<MTLDevice>)m_device.GetMTLDevice();
                s_default = [dev newSamplerStateWithDescriptor:sd];
            }
            samp = s_default;
        }
        [encoder setFragmentSamplerState:samp atIndex:smp_index];
    }

    // --- Draw ---
    if (draw.vertex_array) {
        auto ib = draw.vertex_array->GetIndexBuffer();
        if (ib) {
            auto mtl_ib = std::static_pointer_cast<metal::IndexBuffer>(ib);
            if (mtl_ib->GetMTLBuffer()) {
                id<MTLBuffer> ibuf = (__bridge id<MTLBuffer>)mtl_ib->GetMTLBuffer();
                MTLIndexType idx_type = ToMTLIndexType(ib->GetDataType());
                size_t idx_size = (ib->GetDataType() == IndexBufferDataType::UnsignedInt) ? 4 : 2;
                [encoder drawIndexedPrimitives:ToMTL(prim_type)
                                    indexCount:count
                                     indexType:idx_type
                                   indexBuffer:ibuf
                             indexBufferOffset:offset * idx_size];
            }
        } else {
            [encoder drawPrimitives:ToMTL(prim_type)
                        vertexStart:offset
                        vertexCount:count];
        }
    }
}

void Context::Draw(PrimitiveType prim_type, const DrawState& draw,
                   const void* scene)
{
    if (!draw.vertex_array) return;

    auto ib = draw.vertex_array->GetIndexBuffer();
    if (ib) {
        auto mtl_ib = std::static_pointer_cast<metal::IndexBuffer>(ib);
        int count = static_cast<int>(mtl_ib->GetCount());
        Draw(prim_type, 0, count, draw, scene);
    } else {
        // Non-indexed: the vertex count is the buffer size divided by a vertex's
        // stride. VertexBuffer::GetVertexCount() is never populated (the GL backend
        // doesn't use it either -- it derives the count from size/stride here too),
        // so compute it the same way; otherwise count stays 0 and nothing draws
        // (e.g. a brepkit box mesh, which has no index buffer).
        auto vb = draw.vertex_array->GetVertexBuffer();
        int count = 0;
        if (vb) {
            int stride = 0;
            for (auto& a : draw.vertex_array->GetVertexBufferAttrs()) {
                if (a) { stride = a->GetStrideInBytes(); break; }
            }
            if (stride > 0) {
                count = vb->GetSizeInBytes() / stride;
            }
        }
        Draw(prim_type, 0, count, draw, scene);
    }
}

// ===========================================================================
// Compute
// ===========================================================================

void Context::Compute(const DrawState& draw, int num_groups_x,
                      int num_groups_y, int num_groups_z)
{
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_device.GetCommandQueue();
    id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [cmdBuf computeCommandEncoder];

    // TODO: set compute pipeline state from draw.program
    MTLSize tpg = MTLSizeMake(num_groups_x, num_groups_y, num_groups_z);
    MTLSize tpt = MTLSizeMake(1, 1, 1); // TODO: query from pipeline
    [encoder dispatchThreadgroups:tpg threadsPerThreadgroup:tpt];

    [encoder endEncoding];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];
}

// ===========================================================================
// Viewport
// ===========================================================================

void Context::SetViewport(int x, int y, int w, int h)
{
    m_viewport = Rectangle(x, y, w, h);

    if (m_render_encoder) {
        id<MTLRenderCommandEncoder> encoder =
            (__bridge id<MTLRenderCommandEncoder>)m_render_encoder;
        MTLViewport vp;
        vp.originX = x; vp.originY = y;
        vp.width = w; vp.height = h;
        vp.znear = 0.0; vp.zfar = 1.0;
        [encoder setViewport:vp];
    }
}

void Context::GetViewport(int& x, int& y, int& w, int& h) const
{
    x = m_viewport.x; y = m_viewport.y;
    w = m_viewport.w; h = m_viewport.h;
}

// ===========================================================================
// Texture / sampler binding
// ===========================================================================

void Context::SetTexture(size_t slot, const ur::TexturePtr& tex)
{
    if (slot < MAX_SLOTS) m_bound_textures[slot] = tex;
}

void Context::SetTextureSampler(size_t slot,
    const std::shared_ptr<ur::TextureSampler>& sampler)
{
    if (slot < MAX_SLOTS) m_bound_samplers[slot] = sampler;
}

void Context::SetImage(size_t slot, const ur::TexturePtr& tex, AccessType access)
{
    // TODO: for compute shaders
}

// ===========================================================================
// Framebuffer
// ===========================================================================

void Context::SetFramebuffer(const std::shared_ptr<ur::Framebuffer>& fb)
{
    m_set_framebuffer = fb;

    // Resolve the FBO's color attachments (in Color0..ColorN order, for MRT --
    // e.g. the deferred GBuffer) and its depth attachment (a depth texture or a
    // depth render buffer). A single-attachment FBO (the dtex glyph atlas) is the
    // common case and falls out as colorCount == 1.
    std::array<void*, MAX_COLOR_ATTACH> colorTexs = {};
    size_t colorCount = 0;
    void* depthTex = nullptr;
    if (fb) {
        auto mfb = std::static_pointer_cast<metal::Framebuffer>(fb);
        for (auto& att : mfb->GetAttachments()) {
            int idx = (int)att.type - (int)ur::AttachmentType::Color0;
            if (idx >= 0 && idx < (int)MAX_COLOR_ATTACH) {
                if (att.tex) {
                    colorTexs[idx] = std::static_pointer_cast<metal::Texture>(att.tex)->GetMTLTexture();
                    if ((size_t)(idx + 1) > colorCount) { colorCount = (size_t)(idx + 1); }
                }
            } else if (att.type == ur::AttachmentType::Depth ||
                       att.type == ur::AttachmentType::Stencil) {
                if (att.tex) {
                    depthTex = std::static_pointer_cast<metal::Texture>(att.tex)->GetMTLTexture();
                } else if (att.rbo) {
                    depthTex = std::static_pointer_cast<metal::RenderBuffer>(att.rbo)->GetMTLTexture();
                }
            }
        }
    }

    // One command buffer holds both offscreen (atlas/GBuffer) and screen passes
    // this frame; Metal executes them in submission order, so an FBO rendered
    // here is ready for a later screen pass that samples it -- no CPU sync needed.
    if (colorCount > 0 && colorTexs[0]) {
        EnsureCmdBuffer();
        m_cur_is_fbo = true;
        BeginColorPass(colorTexs.data(), colorCount, depthTex, /*clear*/false);
    } else if (m_frame_active && m_drawable) {
        // Back to the screen drawable mid-frame; keep existing content.
        id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)m_drawable;
        void* color = (__bridge void*)drawable.texture;
        m_cur_is_fbo = false;
        BeginColorPass(&color, 1, m_depth_texture, /*clear*/false);
    } else {
        // Unbinding an FBO before any screen frame has begun: just end the pass;
        // the screen pass starts later on the first on-screen draw (BeginFrame).
        EndEncoder();
        m_cur_color_count  = 0;
        m_cur_depth_target = nullptr;
        m_cur_has_depth    = false;
    }
}

std::shared_ptr<ur::Framebuffer> Context::GetFramebuffer() const
{
    return m_set_framebuffer;
}

// ===========================================================================
// Flush / Pipeline / Barrier
// ===========================================================================

void Context::Flush()
{
    EndFrame();
}

std::shared_ptr<ur::Pipeline>
Context::CreatePipeline(bool include_depth, bool include_vi,
                        const ur::PipelineLayout& layout,
                        const ur::VertexBuffer& vb,
                        const ur::ShaderProgram& prog) const
{
    // TODO: metal::Pipeline wrapping MTLRenderPipelineState
    return nullptr;
}

void Context::SetMemoryBarrier(const std::vector<BarrierType>& types)
{
#if TARGET_OS_OSX
    if (m_render_encoder) {
        id<MTLRenderCommandEncoder> encoder =
            (__bridge id<MTLRenderCommandEncoder>)m_render_encoder;
        [encoder memoryBarrierWithScope:MTLBarrierScopeBuffers | MTLBarrierScopeTextures
                            afterStages:MTLRenderStageVertex | MTLRenderStageFragment
                           beforeStages:MTLRenderStageVertex | MTLRenderStageFragment];
    }
#endif
}

// ===========================================================================
// Pipeline state cache (simplified)
// ===========================================================================

void* Context::GetOrCreatePipelineState(const DrawState& draw)
{
    if (!draw.program) return nullptr;

    auto mtl_prog = std::static_pointer_cast<metal::ShaderProgram>(draw.program);
    if (!mtl_prog->GetVertexFunction() || !mtl_prog->GetFragmentFunction()) {
        return nullptr;
    }
    const int vtxIdx = mtl_prog->GetVertexBufferIndex();

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device.GetMTLDevice();

    MTLRenderPipelineDescriptor* pd = [[MTLRenderPipelineDescriptor alloc] init];
    // Offscreen FBO passes use the clip-space-y-flipped vertex function so the
    // rendered texture matches OpenGL's bottom-left origin (which the engine's UV
    // math assumes); the screen pass uses the normal one.
    pd.vertexFunction   = (__bridge id<MTLFunction>)(m_cur_is_fbo
        ? mtl_prog->GetVertexFunctionFlipped() : mtl_prog->GetVertexFunction());
    pd.fragmentFunction = (__bridge id<MTLFunction>)mtl_prog->GetFragmentFunction();
    // Match the pipeline's color attachment formats to the CURRENT render targets,
    // one per MRT output (the screen drawable is a single BGRA8 target; the
    // deferred GBuffer binds several). A mismatch -- or a fragment shader that
    // writes an output with no matching attachment -- makes pipeline creation fail.
    if (m_cur_color_count == 0) {
        pd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    }
    for (size_t i = 0; i < m_cur_color_count; ++i) {
        id<MTLTexture> ct = (__bridge id<MTLTexture>)m_cur_color_targets[i];
        pd.colorAttachments[i].pixelFormat = ct ? ct.pixelFormat : MTLPixelFormatInvalid;
    }
    // Depth/stencil format must match the active depth target: the screen depth is
    // Depth32Float_Stencil8; an FBO depth render buffer is plain Depth32Float.
    if (m_cur_has_depth && m_cur_depth_target) {
        id<MTLTexture> dt = (__bridge id<MTLTexture>)m_cur_depth_target;
        pd.depthAttachmentPixelFormat = dt.pixelFormat;
        if (dt.pixelFormat == MTLPixelFormatDepth32Float_Stencil8 ||
            dt.pixelFormat == MTLPixelFormatDepth24Unorm_Stencil8) {
            pd.stencilAttachmentPixelFormat = dt.pixelFormat;
        } else {
            pd.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
        }
    } else {
        pd.depthAttachmentPixelFormat   = MTLPixelFormatInvalid;
        pd.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
    }

    // Configure vertex descriptor from vertex array attributes
    if (draw.vertex_array) {
        auto attrs = draw.vertex_array->GetVertexBufferAttrs();
        if (!attrs.empty()) {
            MTLVertexDescriptor* vd = [[MTLVertexDescriptor alloc] init];

            for (size_t i = 0; i < attrs.size(); ++i) {
                if (!attrs[i]) continue;
                auto& a = attrs[i];

                MTLVertexFormat vfmt = MTLVertexFormatFloat3; // default
                switch (a->GetCompDataType()) {
                case ComponentDataType::Float:
                    switch (a->GetNumOfComps()) {
                    case 1: vfmt = MTLVertexFormatFloat;  break;
                    case 2: vfmt = MTLVertexFormatFloat2; break;
                    case 3: vfmt = MTLVertexFormatFloat3; break;
                    case 4: vfmt = MTLVertexFormatFloat4; break;
                    }
                    break;
                case ComponentDataType::UnsignedByte:
                    switch (a->GetNumOfComps()) {
                    case 2: vfmt = MTLVertexFormatUChar2Normalized; break;
                    case 4: vfmt = MTLVertexFormatUChar4Normalized; break;
                    default: vfmt = MTLVertexFormatUChar4Normalized; break;
                    }
                    break;
                default:
                    break;
                }

                vd.attributes[a->GetLocation()].format      = vfmt;
                vd.attributes[a->GetLocation()].offset      = a->GetOffsetInBytes();
                vd.attributes[a->GetLocation()].bufferIndex = vtxIdx;
            }

            // Layout for the vertex (stage_in) buffer
            if (!attrs.empty() && attrs[0]) {
                vd.layouts[vtxIdx].stride       = attrs[0]->GetStrideInBytes();
                vd.layouts[vtxIdx].stepRate     = 1;
                vd.layouts[vtxIdx].stepFunction = MTLVertexStepFunctionPerVertex;
            }

            pd.vertexDescriptor = vd;
        }
    }

    // Configure blending per the draw's render state, on every active color
    // attachment. The 2D UI blends (src-alpha / one-minus-src-alpha); the opaque
    // GBuffer pass disables blend so its depth/normal targets are written raw.
    const auto& bs = draw.render_state.blending;
    const size_t blend_n = (m_cur_color_count > 0) ? m_cur_color_count : 1;
    for (size_t i = 0; i < blend_n; ++i) {
        if (!bs.enabled) {
            pd.colorAttachments[i].blendingEnabled = NO;
            continue;
        }
        pd.colorAttachments[i].blendingEnabled = YES;
        if (bs.separately) {
            pd.colorAttachments[i].sourceRGBBlendFactor        = ToMTLBlend(bs.src_rgb);
            pd.colorAttachments[i].destinationRGBBlendFactor   = ToMTLBlend(bs.dst_rgb);
            pd.colorAttachments[i].rgbBlendOperation           = ToMTLBlendOp(bs.rgb_equation);
            pd.colorAttachments[i].sourceAlphaBlendFactor      = ToMTLBlend(bs.src_alpha);
            pd.colorAttachments[i].destinationAlphaBlendFactor = ToMTLBlend(bs.dst_alpha);
            pd.colorAttachments[i].alphaBlendOperation         = ToMTLBlendOp(bs.alpha_equation);
        } else {
            pd.colorAttachments[i].sourceRGBBlendFactor        = ToMTLBlend(bs.src);
            pd.colorAttachments[i].destinationRGBBlendFactor   = ToMTLBlend(bs.dst);
            pd.colorAttachments[i].rgbBlendOperation           = ToMTLBlendOp(bs.equation);
            pd.colorAttachments[i].sourceAlphaBlendFactor      = ToMTLBlend(bs.src);
            pd.colorAttachments[i].destinationAlphaBlendFactor = ToMTLBlend(bs.dst);
            pd.colorAttachments[i].alphaBlendOperation         = ToMTLBlendOp(bs.equation);
        }
    }

    NSError* error = nil;
    id<MTLRenderPipelineState> pso = [device newRenderPipelineStateWithDescriptor:pd error:&error];
    if (!pso) {
        std::cerr << "[Metal] Pipeline creation failed (fbo=" << m_cur_is_fbo
                  << " n=" << m_cur_color_count << "): "
                  << [[error localizedDescription] UTF8String] << "\n";
        return nullptr;
    }

    // NOTE: In production, cache this by (shader, vertex layout, blend state) hash
    // For now, we create fresh each draw -- acceptable for correctness
    return (__bridge_retained void*)pso;
}

}
}
