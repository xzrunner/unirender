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
        dsd.depthCompareFunction = MTLCompareFunctionLess;
        dsd.depthWriteEnabled    = YES;
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

void Context::BeginFrame()
{
    if (m_frame_active) return;
    if (!m_mtl_layer) return;

    CAMetalLayer* layer = (__bridge CAMetalLayer*)m_mtl_layer;
    id<CAMetalDrawable> drawable = [layer nextDrawable];
    if (!drawable) return;
    m_drawable = (__bridge_retained void*)drawable;

    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_device.GetCommandQueue();
    id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
    m_cmd_buffer = (__bridge_retained void*)cmdBuf;

    // Build render pass descriptor
    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor renderPassDescriptor];

    rpd.colorAttachments[0].texture     = drawable.texture;
    rpd.colorAttachments[0].loadAction  = MTLLoadActionClear;
    rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
    rpd.colorAttachments[0].clearColor  =
        MTLClearColorMake(m_clear_state.color.r / 255.0,
                          m_clear_state.color.g / 255.0,
                          m_clear_state.color.b / 255.0,
                          m_clear_state.color.a / 255.0);

    id<MTLTexture> depthTex = (__bridge id<MTLTexture>)m_depth_texture;
    rpd.depthAttachment.texture      = depthTex;
    rpd.depthAttachment.loadAction   = MTLLoadActionClear;
    rpd.depthAttachment.storeAction  = MTLStoreActionDontCare;
    rpd.depthAttachment.clearDepth   = m_clear_state.depth;

    rpd.stencilAttachment.texture      = depthTex;
    rpd.stencilAttachment.loadAction   = MTLLoadActionClear;
    rpd.stencilAttachment.storeAction  = MTLStoreActionDontCare;
    rpd.stencilAttachment.clearStencil = m_clear_state.stencil;

    id<MTLRenderCommandEncoder> encoder = [cmdBuf renderCommandEncoderWithDescriptor:rpd];
    m_render_encoder = (__bridge_retained void*)encoder;

    // Set depth/stencil state
    [encoder setDepthStencilState:(__bridge id<MTLDepthStencilState>)m_depth_stencil_state];

    // Set viewport
    MTLViewport vp;
    vp.originX = m_viewport.x;
    vp.originY = m_viewport.y;
    vp.width   = (m_viewport.w > 0) ? m_viewport.w : m_width;
    vp.height  = (m_viewport.h > 0) ? m_viewport.h : m_height;
    vp.znear   = 0.0;
    vp.zfar    = 1.0;
    [encoder setViewport:vp];

    m_frame_active = true;
}

void Context::EndFrame()
{
    if (!m_frame_active) return;

    if (m_render_encoder) {
        id<MTLRenderCommandEncoder> encoder =
            (__bridge_transfer id<MTLRenderCommandEncoder>)m_render_encoder;
        [encoder endEncoding];
        m_render_encoder = nullptr;
    }

    if (m_cmd_buffer && m_drawable) {
        id<MTLCommandBuffer> cmdBuf    = (__bridge_transfer id<MTLCommandBuffer>)m_cmd_buffer;
        id<CAMetalDrawable>  drawable  = (__bridge_transfer id<CAMetalDrawable>)m_drawable;
        [cmdBuf presentDrawable:drawable];
        [cmdBuf commit];
        m_cmd_buffer = nullptr;
        m_drawable   = nullptr;
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
    if (!m_frame_active) BeginFrame();
    if (!m_render_encoder) return;

    id<MTLRenderCommandEncoder> encoder =
        (__bridge id<MTLRenderCommandEncoder>)m_render_encoder;

    // --- Bind pipeline state ---
    void* pso = GetOrCreatePipelineState(draw);
    if (pso) {
        [encoder setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)pso];
    }

    // --- Bind vertex buffer ---
    if (draw.vertex_array) {
        auto vb = draw.vertex_array->GetVertexBuffer();
        if (vb) {
            auto mtl_vb = std::static_pointer_cast<metal::VertexBuffer>(vb);
            if (mtl_vb->GetMTLBuffer()) {
                id<MTLBuffer> buf = (__bridge id<MTLBuffer>)mtl_vb->GetMTLBuffer();
                [encoder setVertexBuffer:buf offset:0 atIndex:0];
            }
        }
    }

    // --- Bind textures & samplers ---
    for (size_t i = 0; i < MAX_SLOTS; ++i) {
        if (m_bound_textures[i]) {
            auto mtl_tex = std::static_pointer_cast<metal::Texture>(m_bound_textures[i]);
            if (mtl_tex->GetMTLTexture()) {
                id<MTLTexture> t = (__bridge id<MTLTexture>)mtl_tex->GetMTLTexture();
                [encoder setFragmentTexture:t atIndex:i];
            }
        }
        if (m_bound_samplers[i]) {
            auto mtl_s = std::static_pointer_cast<metal::TextureSampler>(m_bound_samplers[i]);
            if (mtl_s->GetMTLSamplerState()) {
                id<MTLSamplerState> s = (__bridge id<MTLSamplerState>)mtl_s->GetMTLSamplerState();
                [encoder setFragmentSamplerState:s atIndex:i];
            }
        }
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

    id<MTLDevice> device = (__bridge id<MTLDevice>)m_device.GetMTLDevice();

    MTLRenderPipelineDescriptor* pd = [[MTLRenderPipelineDescriptor alloc] init];
    pd.vertexFunction   = (__bridge id<MTLFunction>)mtl_prog->GetVertexFunction();
    pd.fragmentFunction = (__bridge id<MTLFunction>)mtl_prog->GetFragmentFunction();
    pd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pd.depthAttachmentPixelFormat      = MTLPixelFormatDepth32Float_Stencil8;
    pd.stencilAttachmentPixelFormat    = MTLPixelFormatDepth32Float_Stencil8;

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
                vd.attributes[a->GetLocation()].bufferIndex = 0;
            }

            // Layout for buffer index 0
            if (!attrs.empty() && attrs[0]) {
                vd.layouts[0].stride       = attrs[0]->GetStrideInBytes();
                vd.layouts[0].stepRate     = 1;
                vd.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
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
