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
    if (m_depth_stencil_state) { CFRelease(m_depth_stencil_state); }
    if (m_depth_texture)       { CFRelease(m_depth_texture); }
    // m_mtl_layer is bridged from the view, not owned
}

// ===========================================================================
// Init
// ===========================================================================

void Context::Init(void* hwnd, uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

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

    // --- Depth/Stencil state ---
    {
        MTLDepthStencilDescriptor* dsd = [[MTLDepthStencilDescriptor alloc] init];
        // 2D editor: draw order defines layering, so never reject on depth.
        dsd.depthCompareFunction = MTLCompareFunctionAlways;
        dsd.depthWriteEnabled    = NO;
        id<MTLDepthStencilState> dss = [device newDepthStencilStateWithDescriptor:dsd];
        m_depth_stencil_state = (__bridge_retained void*)dss;
    }

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

void Context::BeginColorPass(void* color_tex, bool with_depth, bool clear)
{
    EndEncoder();

    id<MTLCommandBuffer> cmdBuf  = (__bridge id<MTLCommandBuffer>)m_cmd_buffer;
    id<MTLTexture>       colorTex = (__bridge id<MTLTexture>)color_tex;
    if (!cmdBuf || !colorTex) { return; }

    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];
    rpd.colorAttachments[0].texture     = colorTex;
    rpd.colorAttachments[0].loadAction  = clear ? MTLLoadActionClear : MTLLoadActionLoad;
    rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
    if (clear) {
        rpd.colorAttachments[0].clearColor =
            MTLClearColorMake(m_clear_state.color.r / 255.0,
                              m_clear_state.color.g / 255.0,
                              m_clear_state.color.b / 255.0,
                              m_clear_state.color.a / 255.0);
    }

    if (with_depth) {
        id<MTLTexture> depthTex = (__bridge id<MTLTexture>)m_depth_texture;
        rpd.depthAttachment.texture        = depthTex;
        rpd.depthAttachment.loadAction     = clear ? MTLLoadActionClear : MTLLoadActionLoad;
        rpd.depthAttachment.storeAction    = MTLStoreActionDontCare;
        rpd.depthAttachment.clearDepth     = m_clear_state.depth;
        rpd.stencilAttachment.texture      = depthTex;
        rpd.stencilAttachment.loadAction   = clear ? MTLLoadActionClear : MTLLoadActionLoad;
        rpd.stencilAttachment.storeAction  = MTLStoreActionDontCare;
        rpd.stencilAttachment.clearStencil = m_clear_state.stencil;
    }

    id<MTLRenderCommandEncoder> encoder = [cmdBuf renderCommandEncoderWithDescriptor:rpd];
    m_render_encoder = (__bridge_retained void*)encoder;

    if (with_depth) {
        [encoder setDepthStencilState:(__bridge id<MTLDepthStencilState>)m_depth_stencil_state];
    }

    MTLViewport vp;
    vp.originX = m_viewport.x;
    vp.originY = m_viewport.y;
    vp.width   = (m_viewport.w > 0) ? m_viewport.w : (double)colorTex.width;
    vp.height  = (m_viewport.h > 0) ? m_viewport.h : (double)colorTex.height;
    vp.znear   = 0.0;
    vp.zfar    = 1.0;
    [encoder setViewport:vp];

    m_cur_color_target = color_tex;
    m_cur_has_depth    = with_depth;
}

void Context::EnsureCmdBuffer()
{
    if (!m_cmd_buffer) {
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
    BeginColorPass((__bridge void*)drawable.texture, /*with_depth*/true, /*clear*/true);
}

void Context::EndFrame()
{
    EndEncoder();
    m_cur_color_target = nullptr;

    // Commit whatever was recorded this frame (atlas passes and/or screen passes).
    if (m_cmd_buffer) {
        id<MTLCommandBuffer> cmdBuf = (__bridge_transfer id<MTLCommandBuffer>)m_cmd_buffer;
        if (m_drawable) {
            id<CAMetalDrawable> drawable = (__bridge_transfer id<CAMetalDrawable>)m_drawable;
            [cmdBuf presentDrawable:drawable];
            m_drawable = nullptr;
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
    // Values applied at next BeginFrame
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
        auto vb = draw.vertex_array->GetVertexBuffer();
        int count = vb ? static_cast<int>(vb->GetVertexCount()) : 0;
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

    // Resolve the FBO's first color-attachment texture (if any).
    void* colorTex = nullptr;
    if (fb) {
        auto mfb = std::static_pointer_cast<metal::Framebuffer>(fb);
        for (auto& att : mfb->GetAttachments()) {
            if (att.tex) {
                colorTex = std::static_pointer_cast<metal::Texture>(att.tex)->GetMTLTexture();
                break;
            }
        }
    }

    // One command buffer holds both offscreen (atlas) and screen passes this
    // frame; Metal executes them in submission order, so an atlas rendered here
    // is ready for a later screen pass that samples it -- no CPU sync needed.
    if (colorTex) {
        EnsureCmdBuffer();
        BeginColorPass(colorTex, /*with_depth*/false, /*clear*/false);
    } else if (m_frame_active && m_drawable) {
        // Back to the screen drawable mid-frame; keep existing content.
        id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)m_drawable;
        BeginColorPass((__bridge void*)drawable.texture, /*with_depth*/true, /*clear*/false);
    } else {
        // Unbinding an FBO before any screen frame has begun: just end the pass;
        // the screen pass starts later on the first on-screen draw (BeginFrame).
        EndEncoder();
        m_cur_color_target = nullptr;
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
    pd.vertexFunction   = (__bridge id<MTLFunction>)mtl_prog->GetVertexFunction();
    pd.fragmentFunction = (__bridge id<MTLFunction>)mtl_prog->GetFragmentFunction();
    // Match the pipeline's attachment formats to the CURRENT render target
    // (the screen drawable, or an FBO color texture with no depth).
    id<MTLTexture> curColor = (__bridge id<MTLTexture>)m_cur_color_target;
    pd.colorAttachments[0].pixelFormat = curColor ? curColor.pixelFormat : MTLPixelFormatBGRA8Unorm;
    if (m_cur_has_depth) {
        pd.depthAttachmentPixelFormat   = MTLPixelFormatDepth32Float_Stencil8;
        pd.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
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

    // Configure blending (simple alpha blend by default)
    pd.colorAttachments[0].blendingEnabled             = YES;
    pd.colorAttachments[0].sourceRGBBlendFactor        = MTLBlendFactorSourceAlpha;
    pd.colorAttachments[0].destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
    pd.colorAttachments[0].rgbBlendOperation           = MTLBlendOperationAdd;
    pd.colorAttachments[0].sourceAlphaBlendFactor      = MTLBlendFactorOne;
    pd.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pd.colorAttachments[0].alphaBlendOperation         = MTLBlendOperationAdd;

    NSError* error = nil;
    id<MTLRenderPipelineState> pso = [device newRenderPipelineStateWithDescriptor:pd error:&error];
    if (!pso) {
        std::cerr << "[Metal] Pipeline creation failed: "
                  << [[error localizedDescription] UTF8String] << "\n";
        return nullptr;
    }

    // NOTE: In production, cache this by (shader, vertex layout, blend state) hash
    // For now, we create fresh each draw — acceptable for correctness
    return (__bridge_retained void*)pso;
}

}
}
