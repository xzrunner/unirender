#include "unirender/vulkan/Context.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/CommandBuffer.h"
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/FrameBuffers.h"
#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/PipelineCache.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/PipelineLayout.h"
#include "unirender/vulkan/Pipeline.h"
#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/Framebuffer.h"
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/Surface.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Instance.h"
#include "unirender/vulkan/Texture.h"
#include "unirender/vulkan/Utility.h"
#include "unirender/Adaptor.h"
#include "unirender/DrawState.h"
#include "unirender/VertexArray.h"
#include "unirender/VertexArraySizes.h"

#include <vulkan/vulkan.h>

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>

namespace ur
{
namespace vulkan
{

// ---------------------------------------------------------------------------
// Helper: ur::PrimitiveType -> VkPrimitiveTopology
// ---------------------------------------------------------------------------
static VkPrimitiveTopology ToVkTopology(PrimitiveType pt)
{
    switch (pt)
    {
    case PrimitiveType::Points:         return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case PrimitiveType::Lines:          return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case PrimitiveType::LineStrip:      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case PrimitiveType::Triangles:      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case PrimitiveType::TriangleStrip:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case PrimitiveType::TriangleFan:    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    default:                            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

// ===========================================================================
// Construction / destruction
// ===========================================================================

Context::Context(const ur::Device& device, void* hwnd,
                 uint32_t width, uint32_t height)
    : m_device(static_cast<const vulkan::Device&>(device))
{
    Init(hwnd, width, height);
}

Context::~Context()
{
    auto logic_dev = m_device.m_logic_dev->GetHandler();

    // Wait for all GPU work to finish before destroying resources
    vkDeviceWaitIdle(logic_dev);

    // GPU is idle: free any buffers still queued for deferred destruction, then the
    // per-frame ring and the per-image present semaphores.
    m_device.m_logic_dev->CollectAllRetired();
    DestroyRenderFinishedSemaphores();
    DestroyFrameResources();
}

// ===========================================================================
// Init -- called once from the constructor
// ===========================================================================

void Context::Init(void* hwnd, uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    // ---- 1. Surface -------------------------------------------------------
    m_surface = std::make_shared<Surface>(m_device.m_instance, hwnd);

    // ---- 2. Verify physical device supports this surface ------------------
    auto phy_dev = std::make_shared<PhysicalDevice>(*m_device.m_instance, m_surface.get());
    if (phy_dev->GetHandler() != m_device.m_phy_dev->GetHandler()) {
        throw std::runtime_error("Surface requires a different physical device!");
    }

    // ---- 3. (Re)create logical device with present queue ------------------
    //      The Device may have been created without a surface, so the present
    //      queue may not exist yet.  Recreate if necessary.
    PhysicalDevice::QueueFamilyIndices indices =
        PhysicalDevice::FindQueueFamilies(phy_dev->GetHandler(), m_surface.get());

    if (!m_device.m_logic_dev ||
        m_device.m_logic_dev->GetPresentQueue() == VK_NULL_HANDLE)
    {
        const_cast<Device&>(m_device).m_logic_dev =
            std::make_shared<LogicalDevice>(
                m_device.m_enable_validation_layers,
                *m_device.m_phy_dev, m_surface.get());
        const_cast<Device&>(m_device).m_present_family_id =
            indices.present_family.value();
    }

    // ---- 4. Command pool --------------------------------------------------
    // The pool's queue family MUST be the graphics family the command buffers are
    // submitted to (not a hardcoded 0, which only worked when graphics happened to be
    // family 0). `indices` was just resolved against this surface above.
    m_cmd_pool = std::make_shared<CommandPool>(m_device.m_logic_dev,
        indices.graphics_family.value());
    if (!m_device.m_cmd_pool) {
        const_cast<Device&>(m_device).m_cmd_pool = m_cmd_pool;
    }
    // (Per-frame command buffers are created in CreateFrameResources below.)

    // ---- 5. Swapchain & depth buffer --------------------------------------
    m_swapchain = std::make_shared<Swapchain>(
        m_device.m_logic_dev, *m_device.m_phy_dev, *m_surface, width, height);

    m_depth_buf = std::make_shared<DepthBuffer>(
        m_device.m_logic_dev, *m_device.m_phy_dev, width, height);

    m_include_depth = true;

    // ---- 6. Standard descriptor set layouts / pipeline layouts -------------
    SetupDescriptorLayouts();

    // ---- 7. RenderPass, FrameBuffers, PipelineCache -----------------------
    m_renderpass     = std::make_shared<RenderPass>(*this, m_include_depth, /*clear=*/true);
    // LOAD variant: used when the screen pass is resumed after an offscreen pass
    // mid-frame, so the already-drawn screen content is preserved (not re-cleared).
    m_renderpass_load = std::make_shared<RenderPass>(*this, m_include_depth, /*clear=*/false,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    m_frame_buffers  = std::make_shared<FrameBuffers>(*this, m_include_depth);
    m_pipeline_cache = std::make_shared<PipelineCache>(m_device.m_logic_dev);

    // ---- 8. Frames-in-flight ring + per-image present semaphores ----------
    CreateFrameResources();
    CreateRenderFinishedSemaphores();
}

// ---------------------------------------------------------------------------
// Frames-in-flight ring lifecycle
// ---------------------------------------------------------------------------

void Context::CreateFrameResources()
{
    auto vk_dev = m_device.m_logic_dev->GetHandler();

    // Per-frame descriptor pool: sized for a whole frame's worth of per-draw sets
    // (reset once per frame, one set allocated per draw). Mirrors the device pool in
    // SetupDescriptorLayouts -- must cover every descriptor type the reflected sets
    // use (HLSL Texture2D + SamplerState reflect as SEPARATE SampledImage + Sampler).
    std::vector<std::pair<DescriptorType, size_t>> pool_sizes = {
        { DescriptorType::UniformBuffer,        4096 },
        { DescriptorType::CombinedImageSampler, 4096 },
        { DescriptorType::SampledImage,         4096 },
        { DescriptorType::Sampler,              4096 },
        { DescriptorType::StorageImage,         1024 },
        { DescriptorType::StorageBuffer,        1024 },
    };

    VkSemaphoreCreateInfo sem_ci = {};
    sem_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_ci = {};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signalled so frame 0 doesn't block

    for (auto& f : m_frames)
    {
        f.cmd_buf = std::make_shared<CommandBuffer>(m_device.m_logic_dev, m_cmd_pool);

        VkResult fr = vkCreateFence(vk_dev, &fence_ci, nullptr, &f.in_flight);
        assert(fr == VK_SUCCESS);
        VkResult sr = vkCreateSemaphore(vk_dev, &sem_ci, nullptr, &f.image_available);
        assert(sr == VK_SUCCESS);

        f.desc_pool = std::static_pointer_cast<vulkan::DescriptorPool>(
            m_device.CreateDescriptorPool(4096, pool_sizes));
    }
}

void Context::DestroyFrameResources()
{
    auto vk_dev = m_device.m_logic_dev->GetHandler();
    for (auto& f : m_frames)
    {
        if (f.in_flight != VK_NULL_HANDLE) {
            vkDestroyFence(vk_dev, f.in_flight, nullptr);
            f.in_flight = VK_NULL_HANDLE;
        }
        if (f.image_available != VK_NULL_HANDLE) {
            vkDestroySemaphore(vk_dev, f.image_available, nullptr);
            f.image_available = VK_NULL_HANDLE;
        }
        f.cmd_buf.reset();
        f.desc_pool.reset();
    }
}

void Context::CreateRenderFinishedSemaphores()
{
    auto vk_dev = m_device.m_logic_dev->GetHandler();
    VkSemaphoreCreateInfo sem_ci = {};
    sem_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    const uint32_t count = m_swapchain ? m_swapchain->GetImageCount() : 0;
    m_render_finished.assign(count, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < count; ++i) {
        VkResult r = vkCreateSemaphore(vk_dev, &sem_ci, nullptr, &m_render_finished[i]);
        assert(r == VK_SUCCESS);
    }
}

void Context::DestroyRenderFinishedSemaphores()
{
    auto vk_dev = m_device.m_logic_dev->GetHandler();
    for (auto& s : m_render_finished) {
        if (s != VK_NULL_HANDLE) { vkDestroySemaphore(vk_dev, s, nullptr); }
    }
    m_render_finished.clear();
}

VkCommandBuffer Context::CurCmd() const
{
    return m_frames[m_frame_slot].cmd_buf->GetHandler();
}

// ---------------------------------------------------------------------------
// Setup commonly-used descriptor set layouts
// ---------------------------------------------------------------------------
void Context::SetupDescriptorLayouts()
{
    auto& dev = const_cast<Device&>(m_device);

    std::vector<std::pair<DescriptorType, ShaderType>> single_ubo = {
        { DescriptorType::UniformBuffer, ShaderType::VertexShader }
    };
    dev.SetDescriptorSetLayout("single_ubo",
        m_device.CreateDescriptorSetLayout(single_ubo));

    std::vector<std::pair<DescriptorType, ShaderType>> single_img = {
        { DescriptorType::CombinedImageSampler, ShaderType::FragmentShader }
    };
    dev.SetDescriptorSetLayout("single_img",
        m_device.CreateDescriptorSetLayout(single_img));

    std::vector<std::pair<DescriptorType, ShaderType>> ubo_img = {
        { DescriptorType::UniformBuffer,        ShaderType::VertexShader },
        { DescriptorType::CombinedImageSampler, ShaderType::FragmentShader }
    };
    dev.SetDescriptorSetLayout("single_ubo_single_img",
        m_device.CreateDescriptorSetLayout(ubo_img));

    std::vector<std::shared_ptr<ur::DescriptorSetLayout>> layouts = {
        m_device.GetDescriptorSetLayout("single_ubo_single_img")
    };
    dev.SetPipelineLayout("single_ubo_single_img",
        std::make_shared<PipelineLayout>(m_device.m_logic_dev, layouts));

    // Descriptor pool the engine allocates all its descriptor sets from. Nobody
    // created one before (GetDescriptorPool() returned null -> the Vulkan
    // DescriptorSet ctor dereferenced a null pool and crashed). Size it generously
    // for the editor's many renderers (sprite/text/shapes/...).
    // Must cover EVERY descriptor type the reflected sets use. HLSL Texture2D +
    // SamplerState (via DXC) reflect as SEPARATE SampledImage + Sampler (not a
    // CombinedImageSampler), so a pool with only UBO+CombinedImageSampler made
    // vkAllocateDescriptorSets fail (VK_ERROR_OUT_OF_POOL_MEMORY) -> the set wasn't
    // bound -> "missing buffer binding at index 0 for u_mvp" -> nothing rendered.
    std::vector<std::pair<DescriptorType, size_t>> pool_sizes = {
        { DescriptorType::UniformBuffer,        4096 },
        { DescriptorType::CombinedImageSampler, 4096 },
        { DescriptorType::SampledImage,         4096 },
        { DescriptorType::Sampler,              4096 },
        { DescriptorType::StorageImage,         1024 },
        { DescriptorType::StorageBuffer,        1024 },
    };
    dev.SetDescriptorPool(m_device.CreateDescriptorPool(4096, pool_sizes));
}

// ===========================================================================
// Resize
// ===========================================================================

void Context::Resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) return;

    m_width  = width;
    m_height = height;

    auto vk_dev = m_device.m_logic_dev->GetHandler();
    vkDeviceWaitIdle(vk_dev);

    // GPU is idle: free deferred buffers, and drop pipelines built against the OLD
    // screen render pass (they would be render-pass-incompatible if the surface
    // format/extent assumptions changed). They are rebuilt lazily on next use.
    m_device.m_logic_dev->CollectAllRetired();
    m_prog_pipelines.clear();

    // Per-image present semaphores depend on the swapchain image count -- recreate.
    DestroyRenderFinishedSemaphores();

    // Destroy in reverse order, then recreate
    m_frame_buffers.reset();
    m_renderpass.reset();
    m_renderpass_load.reset();
    m_depth_buf.reset();
    m_swapchain.reset();

    m_swapchain = std::make_shared<Swapchain>(
        m_device.m_logic_dev, *m_device.m_phy_dev, *m_surface, width, height);
    m_depth_buf = std::make_shared<DepthBuffer>(
        m_device.m_logic_dev, *m_device.m_phy_dev, width, height);
    m_renderpass      = std::make_shared<RenderPass>(*this, m_include_depth, /*clear=*/true);
    m_renderpass_load = std::make_shared<RenderPass>(*this, m_include_depth, /*clear=*/false,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    m_frame_buffers   = std::make_shared<FrameBuffers>(*this, m_include_depth);

    CreateRenderFinishedSemaphores();

    // Per-frame command buffers / fences / descriptor pools survive a resize: after
    // the device wait-idle every in_flight fence is signalled and every command
    // buffer is free to re-record. Just drop any in-progress frame state.
    m_frame_active = false;
}

// ===========================================================================
// Clear -- store values; applied at render-pass begin
// ===========================================================================

void Context::Clear(const ClearState& clear_state)
{
    m_clear_flag = clear_state.buffers;

    const bool clear_color =
        static_cast<uint32_t>(m_clear_flag) & static_cast<uint32_t>(ClearBuffers::ColorBuffer);
    if (clear_color) {
        m_clear_color = clear_state.color;
    }
    if (static_cast<uint32_t>(m_clear_flag) & static_cast<uint32_t>(ClearBuffers::DepthBuffer)) {
        m_clear_depth = clear_state.depth;
    }
    if (static_cast<uint32_t>(m_clear_flag) & static_cast<uint32_t>(ClearBuffers::StencilBuffer)) {
        m_clear_stencil = clear_state.stencil;
    }

    // The screen clear is deferred to the first screen pass (m_clear_color), but an
    // OFFSCREEN FBO (the GBuffer) must be cleared NOW like GL/Metal do -- our offscreen
    // passes use loadOp=LOAD, so without this the GBuffer keeps its creation-time clear
    // and the 3D viewport background stays black instead of the rendergraph's gray.
    if (clear_color && m_set_framebuffer) {
        auto* fbo = static_cast<vulkan::Framebuffer*>(m_set_framebuffer.get());
        ClearFboColor(*fbo, m_clear_color);
    }
}

// Direct color clear of an offscreen FBO's color attachment images (SHADER_READ_ONLY ->
// TRANSFER_DST -> vkCmdClearColorImage -> SHADER_READ_ONLY). Used by Clear() so the
// GBuffer gets its per-frame (gray) background.
void Context::ClearFboColor(const vulkan::Framebuffer& fbo, const Color& color)
{
    auto vk_dev = m_device.m_logic_dev->GetHandler();
    auto pool   = m_device.m_cmd_pool->GetHandler();
    auto queue  = m_device.m_logic_dev->GetGraphicsQueue();

    VkClearColorValue cc = {};
    cc.float32[0] = color.r / 255.0f;
    cc.float32[1] = color.g / 255.0f;
    cc.float32[2] = color.b / 255.0f;
    cc.float32[3] = color.a / 255.0f;
    VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    for (auto& a : fbo.GetAttachments())
    {
        if (a.type < AttachmentType::Color0 || a.type > AttachmentType::Color15 || !a.tex) {
            continue;
        }
        VkImage img = std::static_pointer_cast<vulkan::Texture>(a.tex)->GetVkImage();
        if (img == VK_NULL_HANDLE) { continue; }

        VkCommandBuffer cb = CommandBuffer::BeginSingleTimeCommands(vk_dev, pool);

        VkImageMemoryBarrier to_dst = {};
        to_dst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        to_dst.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        to_dst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        to_dst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        to_dst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        to_dst.image = img;
        to_dst.subresourceRange = range;
        to_dst.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        to_dst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &to_dst);

        vkCmdClearColorImage(cb, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cc, 1, &range);

        VkImageMemoryBarrier to_read = to_dst;
        to_read.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        to_read.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        to_read.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        to_read.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &to_read);

        CommandBuffer::EndSingleTimeCommands(cb, vk_dev, pool, queue);
    }
}

// ===========================================================================
// Draw (public overloads)
// ===========================================================================

void Context::Draw(PrimitiveType prim_type, int offset, int count,
                   const DrawState& draw, const void* scene)
{
    DrawImpl(draw, prim_type, offset, count);
}

void Context::Draw(PrimitiveType prim_type, const DrawState& draw,
                   const void* scene)
{
    if (!draw.vertex_array) return;

    auto ib = draw.vertex_array->GetIndexBuffer();
    if (ib) {
        DrawImpl(draw, prim_type, 0, static_cast<int>(ib->GetCount()));
    } else {
        auto vb = draw.vertex_array->GetVertexBuffer();
        int count = vb ? static_cast<int>(vb->GetVertexCount()) : 0;
        DrawImpl(draw, prim_type, 0, count);
    }
}

// ===========================================================================
// DrawImpl -- the real rendering pipeline
// ===========================================================================

void Context::DrawImpl(const DrawState& ds, PrimitiveType prim_type,
                       int offset, int count)
{
    if (std::getenv("TT_VK_DRAWALL")) {
        const Framebuffer* dbgfbo = std::static_pointer_cast<vulkan::Framebuffer>(m_set_framebuffer).get();
        std::cerr << "[drawimpl] count=" << count << " prog=" << (ds.program ? 1 : 0)
                  << " va=" << (ds.vertex_array ? 1 : 0)
                  << " fbo=" << (dbgfbo ? "OFF" : "SCR")
                  << (count == 0 ? " SKIP(count0)" : "") << std::endl;
    }
    if (count == 0) return;

    BeginFrameIfNeeded();

    const Framebuffer* fbo = std::static_pointer_cast<vulkan::Framebuffer>(m_set_framebuffer).get();

    // Record this draw into the frame's SINGLE command buffer -- no per-draw submit.
    // The whole frame is submitted once in Flush(). BeginPass switches render pass
    // (screen vs offscreen FBO) only when the target changes; consecutive draws to
    // the same target share one vkCmdBeginRenderPass..vkCmdEndRenderPass instance.
    // Recreating a vertex/index buffer between draws is now safe: the old VkBuffer is
    // retired (deferred-destroyed) rather than freed while still referenced -- see
    // Buffer::Clear / LogicalDevice retirement. Offscreen<->screen and acquire
    // ordering is handled by the render passes' own external subpass dependencies
    // (Framebuffer.cpp / RenderPass.cpp), so no per-draw fence is needed.
    BeginPass(fbo); // lazy-acquires the swapchain image on the first screen pass
    RecordDraw(CurCmd(), ds, prim_type, offset, count);
    // Do NOT end the pass here -- the next draw to the same target reuses it; Flush
    // (or a target switch in BeginPass) closes it.

    // Clear per-draw texture bindings so the next draw only sees what it explicitly
    // binds. m_bound_textures is indexed by slot (= reflected binding for rendergraph
    // passes), so a multi-texture post pass that sets slots 1/2/3 must not leak those
    // into a later draw whose texture descriptor happens to sit at the same binding.
    for (auto& t : m_bound_textures) { t.reset(); }
    for (auto& s : m_bound_samplers) { s.reset(); }
}

void Context::BeginFrameIfNeeded()
{
    if (m_frame_active) return;
    m_frame_active        = true;
    m_swapchain_acquired  = false;
    m_pass_open           = false;
    m_screen_cleared      = false;
    m_cur_pass_fbo        = nullptr;

    m_frame_slot = static_cast<uint32_t>(m_frame_number % MAX_FRAMES_IN_FLIGHT);
    FrameSlot& slot = m_frames[m_frame_slot];
    auto vk_dev = m_device.m_logic_dev->GetHandler();

    // Wait until the GPU has finished this slot's PREVIOUS use (frame
    // m_frame_number - MAX_FRAMES_IN_FLIGHT) before reusing its command buffer,
    // descriptor pool and acquire semaphore. This is the ONLY CPU/GPU sync point now
    // -- once per frame, vs the old once-per-draw stall.
    VkResult wr = vkWaitForFences(vk_dev, 1, &slot.in_flight, VK_TRUE, UINT64_MAX);
    assert(wr == VK_SUCCESS);

    // That previous frame is now done, so any resources it retired (and earlier) are
    // safe to free. completed = m_frame_number - MAX_FRAMES_IN_FLIGHT.
    if (m_frame_number >= MAX_FRAMES_IN_FLIGHT) {
        m_device.m_logic_dev->CollectRetired(m_frame_number - MAX_FRAMES_IN_FLIGHT);
    }
    // Buffers retired from here on belong to THIS frame.
    m_device.m_logic_dev->SetCurrentFrame(m_frame_number);

    vkResetFences(vk_dev, 1, &slot.in_flight);

    // Reset this slot's descriptor pool ONCE per frame (the fence wait above
    // guarantees its sets from the previous cycle are no longer in use), then begin
    // recording the frame into this slot's command buffer.
    vkResetDescriptorPool(vk_dev, slot.desc_pool->GetHandler(), 0);

    VkCommandBufferBeginInfo cb_begin = {};
    cb_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult br = vkBeginCommandBuffer(slot.cmd_buf->GetHandler(), &cb_begin);
    assert(br == VK_SUCCESS);
}

void Context::EndCurrentPass()
{
    if (m_pass_open) {
        vkCmdEndRenderPass(CurCmd());
        m_pass_open = false;
    }
}

void Context::BeginPass(const vulkan::Framebuffer* fbo)
{
    if (m_pass_open && m_cur_pass_fbo == fbo) {
        return; // already recording into the right target
    }
    EndCurrentPass();

    auto cmd_buf = CurCmd();

    VkRenderPass  rp;
    VkFramebuffer fb;
    std::vector<VkClearValue> clears;

    if (fbo)
    {
        const_cast<vulkan::Framebuffer*>(fbo)->EnsureBuilt();
        rp = fbo->GetVkRenderPass();
        fb = fbo->GetVkFramebuffer();
        m_cur_extent = fbo->GetExtent();
        m_cur_color_count = fbo->GetColorAttachmentCount(); // MRT GBuffer = 3
        // Color attachments are LOAD (values unused); the depth attachment (if any) is
        // CLEAR each pass -> provide its clear value at index = color count.
        clears.resize(m_cur_color_count + (fbo->HasDepth() ? 1 : 0));
        if (fbo->HasDepth()) {
            clears[m_cur_color_count].depthStencil = { 1.0f, 0 };
        }

        // Diagnostic: remember EVERY distinct offscreen color target so Flush can
        // dump them all (find where each renderer actually draws).
        if (std::getenv("TT_VK_DUMP")) {
            for (auto& a : fbo->GetAttachments()) {
                if (a.type >= AttachmentType::Color0 && a.type <= AttachmentType::Color15 && a.tex) {
                    auto t = std::static_pointer_cast<vulkan::Texture>(a.tex);
                    VkImage img = t->GetVkImage();
                    bool seen = false;
                    for (auto& d : m_dump_targets) { if (d.image == img) { seen = true; break; } }
                    if (!seen) {
                        m_dump_targets.push_back({ img, t->GetVkFormat(),
                            (uint32_t)t->GetWidth(), (uint32_t)t->GetHeight() });
                    }
                    break;
                }
            }
        }
    }
    else
    {
        // Acquire a swapchain image the first time a screen pass is needed. The
        // acquire signals THIS frame slot's image_available semaphore, which the
        // single frame submit in Flush() waits on at COLOR_ATTACHMENT_OUTPUT.
        if (!m_swapchain_acquired) {
            VkResult ar = vkAcquireNextImageKHR(
                m_device.m_logic_dev->GetHandler(), m_swapchain->GetHandler(), UINT64_MAX,
                m_frames[m_frame_slot].image_available, VK_NULL_HANDLE, &m_current_buffer);
            assert(ar == VK_SUCCESS || ar == VK_SUBOPTIMAL_KHR);
            m_swapchain_acquired = true;
        }
        // Clear the screen once per frame; resume with LOAD if a later screen pass
        // follows an offscreen pass (so earlier screen content is preserved).
        rp = (m_screen_cleared ? m_renderpass_load : m_renderpass)->GetHandler();
        fb = m_frame_buffers->GetHandler(m_current_buffer);
        m_cur_extent = { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) };
        m_cur_color_count = 1; // swapchain is single-color
        clears.resize(m_include_depth ? 2 : 1);
        clears[0].color = {{
            m_clear_color.r / 255.0f, m_clear_color.g / 255.0f,
            m_clear_color.b / 255.0f, m_clear_color.a / 255.0f
        }};
        if (m_include_depth) {
            clears[1].depthStencil = { static_cast<float>(m_clear_depth),
                                       static_cast<uint32_t>(m_clear_stencil) };
        }
        m_screen_cleared = true;
    }

    VkRenderPassBeginInfo rp_begin = {};
    rp_begin.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass        = rp;
    rp_begin.framebuffer       = fb;
    rp_begin.renderArea.offset = {0, 0};
    rp_begin.renderArea.extent = m_cur_extent;
    rp_begin.clearValueCount   = static_cast<uint32_t>(clears.size());
    rp_begin.pClearValues      = clears.data();
    vkCmdBeginRenderPass(cmd_buf, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    // TEMP probe (TT_VK_CLEARDRAW): clear the color attachment to red WITHIN the
    // pass -- independent of vertex/pipeline/shader. If the dumped target turns red,
    // the framebuffer correctly targets that texture and only the draw path is at
    // fault; if it stays black, the framebuffer/attachment is mis-wired.
    if (std::getenv("TT_VK_CLEARDRAW")) {
        VkClearAttachment ca = {};
        ca.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ca.colorAttachment = 0;
        ca.clearValue.color = {{ 1.0f, 0.0f, 0.0f, 1.0f }};
        VkClearRect cr = {};
        cr.rect.offset = { 0, 0 };
        cr.rect.extent = m_cur_extent;
        cr.baseArrayLayer = 0;
        cr.layerCount = 1;
        vkCmdClearAttachments(cmd_buf, 1, &ca, 1, &cr);
    }

    m_pass_open       = true;
    m_cur_pass_fbo    = fbo;
    m_cur_render_pass = rp; // pipelines for this pass must be built against this

    if (std::getenv("TT_VK_FRAME")) {
        std::cerr << "[vkframe] pass " << (fbo ? "OFFSCREEN" : "SCREEN")
                  << " extent=" << m_cur_extent.width << "x" << m_cur_extent.height
                  << " clear=(" << (int)m_clear_color.r << "," << (int)m_clear_color.g
                  << "," << (int)m_clear_color.b << ")"
                  << " acquired=" << (int)m_swapchain_acquired << std::endl;
    }
}

// ===========================================================================
// BuildCommandBuffer -- records one frame
// ===========================================================================

void Context::RecordDraw(VkCommandBuffer cmd_buf, const DrawState& ds,
                         PrimitiveType prim_type, int offset, int count)
{
    auto vk_dev = m_device.m_logic_dev->GetHandler();

    // Run uniform updaters + flush each named uniform into its host-visible UBO.
    // (Draws sharing a ShaderProgram share its UBOs; the editor's 2D path uses
    // ~constant matrices + intense per frame, so batching them is fine.)
    if (ds.program) {
        ds.program->Clean(*this, ds, nullptr);
    }

    // ---- Viewport & scissor (dynamic state), sized to the current pass target ----
    VkViewport viewport = {};
    viewport.x        = static_cast<float>(m_viewport.x);
    viewport.y        = static_cast<float>(m_viewport.y);
    viewport.width    = (m_viewport.w > 0) ? static_cast<float>(m_viewport.w)
                                           : static_cast<float>(m_cur_extent.width);
    viewport.height   = (m_viewport.h > 0) ? static_cast<float>(m_viewport.h)
                                           : static_cast<float>(m_cur_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    // Negative-height viewport flips the screen vertically. Vulkan clip-space Y points
    // the opposite way to the engine's (Metal/GL) convention, so without this the 2D GUI
    // (and mouse-vs-render mapping, e.g. the rubber-band selection) renders upside-down.
    // Screen pass only; the offscreen atlas is packed + sampled in one consistent space.
    if (m_cur_pass_fbo == nullptr) {
        viewport.y      = viewport.y + viewport.height;
        viewport.height = -viewport.height;
    }
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.extent = m_cur_extent;
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

    if (std::getenv("TT_VK_FRAME")) {
        std::cerr << "[vkdraw] " << (m_cur_pass_fbo ? "OFF" : "SCR")
                  << " vp=" << (int)viewport.x << "," << (int)viewport.y
                  << " " << (int)viewport.width << "x" << (int)viewport.height
                  << " count=" << count << " prog=" << (ds.program ? 1 : 0)
                  << " va=" << (ds.vertex_array ? 1 : 0) << " texslots=[";
        for (size_t s = 0; s < m_bound_textures.size(); ++s) {
            if (m_bound_textures[s]) {
                std::cerr << s << ":" << (void*)std::static_pointer_cast<vulkan::Texture>(m_bound_textures[s])->GetVkImage() << " ";
            }
        }
        std::cerr << "]" << std::endl;
    }

    // ---- Build & bind the descriptor set from the program's reflection ----
    // The engine's ds.desc_set / ds.pipeline_layout (hardcoded single_ubo_single_img)
    // can't represent the shader's real bindings, so we build a matching set here
    // from the ShaderProgram's reflection: its reflected UBOs + the engine-bound
    // texture, addressed by the deconflicted bindings.
    auto prog = std::static_pointer_cast<vulkan::ShaderProgram>(ds.program);
    if (prog && prog->GetVkDescriptorSetLayout() != VK_NULL_HANDLE)
    {
        // Allocate this draw's set from the CURRENT frame slot's pool. That pool is
        // reset exactly once per frame in BeginFrameIfNeeded (after its fence wait),
        // so sets allocated here stay valid for the whole frame's execution and are
        // only recycled once the GPU has finished this slot's previous frame.
        auto pool = m_frames[m_frame_slot].desc_pool;
        VkDescriptorSetLayout layout = prog->GetVkDescriptorSetLayout();
        VkDescriptorSetAllocateInfo ai = {};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = pool->GetHandler();
        ai.descriptorSetCount = 1;
        ai.pSetLayouts = &layout;
        VkDescriptorSet dset = VK_NULL_HANDLE;
        if (vkAllocateDescriptorSets(vk_dev, &ai, &dset) == VK_SUCCESS)
        {
            auto& binds = prog->GetDescBindings();
            std::vector<VkWriteDescriptorSet>   writes;
            std::vector<VkDescriptorBufferInfo> buf_infos;
            std::vector<VkDescriptorImageInfo>  img_infos;
            buf_infos.reserve(binds.size());
            img_infos.reserve(binds.size());

            // Pick the texture for a sampler/image descriptor by its BINDING. The
            // rendergraph binds post-pass textures via SetTexture(QueryTexSlot(name), tex),
            // and QueryTexSlot returns the reflected binding -- so m_bound_textures[binding]
            // is that sampler's texture (deferred edge/composite sample main/depth/normal
            // at their distinct bindings). The 2D GUI (SpriteRenderer/easygui) instead binds
            // its single texture to slot 0, so fall back to slot 0 when binding has none.
            auto pick_tex = [&](uint32_t binding) -> std::shared_ptr<ur::Texture> {
                if (binding < m_bound_textures.size() && m_bound_textures[binding]) {
                    return m_bound_textures[binding];
                }
                return m_bound_textures[0];
            };

            for (auto& b : binds)
            {
                VkWriteDescriptorSet w = {};
                w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                w.dstSet = dset;
                w.dstBinding = b.binding;
                w.descriptorCount = 1;
                w.descriptorType = b.type;
                if (b.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && b.ubo) {
                    buf_infos.push_back(b.ubo->GetBufferInfo());
                    w.pBufferInfo = &buf_infos.back();
                } else if (b.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                           b.type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
                           b.type == VK_DESCRIPTOR_TYPE_SAMPLER) {
                    auto t = pick_tex(b.binding);
                    if (!t) { continue; }
                    img_infos.push_back(std::static_pointer_cast<vulkan::Texture>(t)->GetDescInfo());
                    w.pImageInfo = &img_infos.back();
                } else {
                    continue;
                }
                writes.push_back(w);
            }
            if (!writes.empty()) {
                vkUpdateDescriptorSets(vk_dev, (uint32_t)writes.size(), writes.data(), 0, nullptr);
            }
            vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                prog->GetVkPipelineLayout(), 0, 1, &dset, 0, nullptr);
        }
    }

    // ---- Bind pipeline ----
    // IGNORE the engine-provided ds.pipeline: it was created against the screen
    // render pass and is NOT render-pass-compatible with an offscreen pass (different
    // attachment formats / no depth) -- using it offscreen silently draws nothing.
    // Instead build/cache a pipeline against the CURRENT render pass, keyed by
    // (program, target FBO). Offscreen passes have no depth attachment, so disable
    // depth there.
    VkPrimitiveTopology topology = ToVkTopology(prim_type);
    std::shared_ptr<vulkan::Pipeline> pipeline;
    if (prog && ds.vertex_array) {
        auto vb = ds.vertex_array->GetVertexBuffer();
        if (vb) {
            auto key = std::make_tuple((const void*)prog.get(), (const void*)m_cur_pass_fbo, (int)topology);
            auto it = m_prog_pipelines.find(key);
            if (it != m_prog_pipelines.end()) {
                pipeline = it->second;
            } else {
                // Depth: screen uses the swapchain depth buffer; an offscreen FBO uses
                // its own depth attachment (the GBuffer has one -> the 3D mesh depth-tests
                // so near faces occlude far faces instead of rendering see-through).
                bool with_depth = m_cur_pass_fbo
                    ? m_cur_pass_fbo->HasDepth()
                    : m_include_depth;
                if (std::getenv("TT_VK_NODEPTH")) { with_depth = false; } // TEMP probe
                // Cached by (program, fbo, topology): a given program uses one RenderState,
                // so the blend captured at first build is the blend for all its draws; the
                // topology varies (mesh = tri-list, post-process quad = tri-strip).
                pipeline = std::make_shared<vulkan::Pipeline>(
                    *this, with_depth, true, *vb, *prog, ds.render_state.blending, topology);
                m_prog_pipelines[key] = pipeline;
            }
        }
    }
    // A pipeline whose build failed (e.g. a shader with a bad VS->FS interface) has a
    // null handle; skip the whole draw rather than binding/drawing with it.
    if (!pipeline || pipeline->GetHandler() == VK_NULL_HANDLE) {
        return;
    }
    vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandler());

    // Re-issue dynamic viewport/scissor AFTER the pipeline bind. MoltenVK can drop
    // dynamic state set before vkCmdBindPipeline, leaving an empty (0x0) viewport so
    // draws clip away entirely (while vkCmdClearAttachments, which uses the render
    // area, still works) -- which is exactly the symptom we saw.
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

    // ---- Bind vertex / index buffers, draw ----
    if (ds.vertex_array)
    {
        auto vb = ds.vertex_array->GetVertexBuffer();
        auto ib = ds.vertex_array->GetIndexBuffer();

        VkDeviceSize vb_offsets[1] = {0};
        VkBuffer dbg_vb = VK_NULL_HANDLE, dbg_ib = VK_NULL_HANDLE;
        if (vb) {
            auto vk_vb = std::static_pointer_cast<vulkan::VertexBuffer>(vb)->GetBuffer();
            dbg_vb = vk_vb;
            if (vk_vb != VK_NULL_HANDLE) {
                vkCmdBindVertexBuffers(cmd_buf, 0, 1, &vk_vb, vb_offsets);
            }
        }
        if (std::getenv("TT_VK_FRAME")) {
            std::cerr << "[vkbuf] vb=" << (dbg_vb != VK_NULL_HANDLE)
                      << " ib_present=" << (ib ? 1 : 0)
                      << " attribs=" << (vb ? std::static_pointer_cast<vulkan::VertexBuffer>(vb)->GetVertInputAttrDesc().size() : 0)
                      << " stride=" << (vb ? std::static_pointer_cast<vulkan::VertexBuffer>(vb)->GetVertInputBindDesc().stride : 0)
                      << std::endl;
        }

        if (ib) {
            auto vk_ib = std::static_pointer_cast<vulkan::IndexBuffer>(ib)->GetBuffer();
            dbg_ib = vk_ib; (void)dbg_ib;
            // FIX: support both uint16 and uint32 index types
            VkIndexType idx_type = (ib->GetDataType() == IndexBufferDataType::UnsignedInt)
                ? VK_INDEX_TYPE_UINT32
                : VK_INDEX_TYPE_UINT16;
            vkCmdBindIndexBuffer(cmd_buf, vk_ib, 0, idx_type);
            vkCmdDrawIndexed(cmd_buf, static_cast<uint32_t>(count), 1,
                             static_cast<uint32_t>(offset), 0, 0);
        } else {
            vkCmdDraw(cmd_buf, static_cast<uint32_t>(count), 1,
                      static_cast<uint32_t>(offset), 0);
        }
    }
}

// ===========================================================================
// Viewport
// ===========================================================================

void Context::SetViewport(int x, int y, int w, int h)
{
    m_viewport = Rectangle(x, y, w, h);
}

void Context::GetViewport(int& x, int& y, int& w, int& h) const
{
    x = m_viewport.x;
    y = m_viewport.y;
    w = m_viewport.w;
    h = m_viewport.h;
}

// ===========================================================================
// Texture / sampler / image binding -- stored for next draw
// ===========================================================================

void Context::SetTexture(size_t slot, const ur::TexturePtr& tex)
{
    if (slot < m_bound_textures.size()) {
        m_bound_textures[slot] = tex;
    }
}

void Context::SetTextureSampler(size_t slot,
    const std::shared_ptr<ur::TextureSampler>& sampler)
{
    if (slot < m_bound_samplers.size()) {
        m_bound_samplers[slot] = sampler;
    }
}

void Context::SetImage(size_t slot, const ur::TexturePtr& tex, AccessType access)
{
    // TODO: storage image binding for compute shaders
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
// Pixel transfer state -- N/A for Vulkan
// ===========================================================================

void Context::SetUnpackRowLength(int len) {}
void Context::SetPackRowLength(int len) {}

// ===========================================================================
// Misc
// ===========================================================================

bool Context::CheckRenderTargetStatus()
{
    return true; // Vulkan validates at pipeline creation
}

void Context::DumpImageToPPM(VkImage image, VkFormat format, uint32_t w, uint32_t h,
                             VkImageLayout cur_layout, const char* path)
{
    if (image == VK_NULL_HANDLE || w == 0 || h == 0) { return; }

    auto vk_dev = m_device.m_logic_dev->GetHandler();
    VkDeviceSize size = (VkDeviceSize)w * h * 4;

    VkBuffer buf = VK_NULL_HANDLE; VkDeviceMemory mem = VK_NULL_HANDLE;
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = size; bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(vk_dev, &bci, nullptr, &buf) != VK_SUCCESS) { return; }
    VkMemoryRequirements mr; vkGetBufferMemoryRequirements(vk_dev, buf, &mr);
    VkMemoryAllocateInfo mai = {};
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = mr.size;
    mai.memoryTypeIndex = Utility::FindMemoryType(m_device.m_phy_dev->GetHandler(), mr.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkAllocateMemory(vk_dev, &mai, nullptr, &mem);
    vkBindBufferMemory(vk_dev, buf, mem, 0);

    VkCommandBuffer cb = CommandBuffer::BeginSingleTimeCommands(vk_dev, m_device.m_cmd_pool->GetHandler());
    VkImageMemoryBarrier b = {};
    b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    b.oldLayout = cur_layout; b.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image; b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    b.srcAccessMask = 0; b.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &b);
    VkBufferImageCopy region = {};
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageExtent = { w, h, 1 };
    vkCmdCopyImageToBuffer(cb, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buf, 1, &region);
    VkImageMemoryBarrier b2 = b;
    b2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; b2.newLayout = cur_layout;
    b2.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT; b2.dstAccessMask = 0;
    vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0, 0, nullptr, 0, nullptr, 1, &b2);
    CommandBuffer::EndSingleTimeCommands(cb, vk_dev, m_device.m_cmd_pool->GetHandler(),
        m_device.m_logic_dev->GetGraphicsQueue());

    void* data = nullptr;
    vkMapMemory(vk_dev, mem, 0, size, 0, &data);
    auto* px = static_cast<unsigned char*>(data);
    FILE* f = fopen(path, "wb");
    if (f) {
        fprintf(f, "P6\n%u %u\n255\n", w, h);
        const bool bgra = (format == VK_FORMAT_B8G8R8A8_UNORM);
        unsigned long nonzero = 0;
        for (uint32_t i = 0; i < w * h; ++i) {
            unsigned char r, g, bl;
            if (bgra) { bl = px[i*4+0]; g = px[i*4+1]; r = px[i*4+2]; }
            else      { r = px[i*4+0]; g = px[i*4+1]; bl = px[i*4+2]; }
            if (r || g || bl) { ++nonzero; }
            fputc(r, f); fputc(g, f); fputc(bl, f);
        }
        fclose(f);
        std::cerr << "[vkdump] wrote " << path << " " << w << "x" << h
                  << " nonzero_px=" << nonzero << " image=" << (void*)image << std::endl;
    }
    vkUnmapMemory(vk_dev, mem);
    vkDestroyBuffer(vk_dev, buf, nullptr);
    vkFreeMemory(vk_dev, mem, nullptr);
}

void Context::Flush()
{
    // End-of-frame: finish recording the frame's single command buffer, submit it
    // ONCE, and present. Called from main.cpp after the whole frame is drawn.
    if (!m_frame_active) {
        return; // nothing was drawn this frame
    }

    auto vk_dev         = m_device.m_logic_dev->GetHandler();
    auto graphics_queue = m_device.m_logic_dev->GetGraphicsQueue();
    FrameSlot& slot     = m_frames[m_frame_slot];

    // Close any render pass left open by the last draw, then finish recording.
    EndCurrentPass();
    VkCommandBuffer cb = slot.cmd_buf->GetHandler();
    VkResult er = vkEndCommandBuffer(cb);
    assert(er == VK_SUCCESS);

    // Submit the WHOLE frame in one go. If a screen pass acquired an image, wait its
    // acquire semaphore at color-output and signal that image's render-finished
    // semaphore (which present waits on -- this is also what orders present after
    // render for MoltenVK, replacing the old empty signalling submit). The fence
    // lets a later frame reusing this slot know when the GPU is done with it.
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si = {};
    si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &cb;
    if (m_swapchain_acquired) {
        si.waitSemaphoreCount   = 1;
        si.pWaitSemaphores      = &slot.image_available;
        si.pWaitDstStageMask    = &wait_stage;
        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores    = &m_render_finished[m_current_buffer];
    }
    VkResult sr = vkQueueSubmit(graphics_queue, 1, &si, slot.in_flight);
    assert(sr == VK_SUCCESS);

    // Diagnostic dump (TT_VK_DUMP=<frame>): the new model does NOT idle the GPU per
    // frame, so wait here (debug path only) before reading the targets.
    ++m_frame_count;
    if (const char* d = std::getenv("TT_VK_DUMP")) {
        if (m_frame_count == atoi(d)) {
            vkDeviceWaitIdle(vk_dev);
            for (size_t i = 0; i < m_dump_targets.size(); ++i) {
                char path[64];
                snprintf(path, sizeof(path), "/tmp/vk_off%zu_%ux%u.ppm",
                         i, m_dump_targets[i].w, m_dump_targets[i].h);
                DumpImageToPPM(m_dump_targets[i].image, m_dump_targets[i].fmt,
                               m_dump_targets[i].w, m_dump_targets[i].h,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, path);
            }
            if (m_swapchain_acquired) {
                DumpImageToPPM(m_swapchain->GetImage((int)m_current_buffer),
                               VK_FORMAT_B8G8R8A8_UNORM, (uint32_t)m_width, (uint32_t)m_height,
                               VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, "/tmp/vk_swapchain.ppm");
            }
        }
    }

    if (std::getenv("TT_VK_FRAME")) {
        std::cerr << "[vkframe] FLUSH present acquired=" << (int)m_swapchain_acquired
                  << " buffer=" << m_current_buffer << std::endl;
    }

    if (m_swapchain_acquired) {
        VkResult present = m_swapchain->QueuePresent(
            graphics_queue, m_current_buffer, m_render_finished[m_current_buffer]);
        if (present == VK_ERROR_OUT_OF_DATE_KHR || present == VK_SUBOPTIMAL_KHR) {
            Resize(m_width, m_height);
        }
    }

    ++m_frame_number; // advance to the next slot for the next frame
    m_frame_active = false;
}

std::shared_ptr<ur::Pipeline>
Context::CreatePipeline(bool include_depth, bool include_vi,
                        const ur::PipelineLayout& layout,
                        const ur::VertexBuffer& vb,
                        const ur::ShaderProgram& prog) const
{
    return std::make_shared<Pipeline>(*this, m_include_depth, include_vi, layout, vb, prog);
}

void Context::SetMemoryBarrier(const std::vector<BarrierType>& types)
{
    // OpenGL glMemoryBarrier equivalent: make prior (typically compute/storage)
    // writes visible to the accesses named in `types`. We record a global
    // VkMemoryBarrier into the frame's command buffer. A pipeline barrier must be
    // OUTSIDE a render pass instance, so close the current pass first; the next draw
    // re-begins the right pass via BeginPass. (Without an active frame there is no
    // command buffer to record into, so this is a no-op then.)
    if (!m_frame_active || types.empty()) {
        return;
    }
    EndCurrentPass();

    VkPipelineStageFlags src_stage = 0, dst_stage = 0;
    VkAccessFlags        src_access = 0, dst_access = 0;
    for (auto t : types)
    {
        switch (t)
        {
        case BarrierType::ShaderImageAccess:
        case BarrierType::ShaderStorage:
            src_stage  |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stage  |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                        | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            src_access |= VK_ACCESS_SHADER_WRITE_BIT;
            dst_access |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            break;
        case BarrierType::TextureFetch:
            src_stage  |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dst_stage  |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            src_access |= VK_ACCESS_SHADER_WRITE_BIT;
            dst_access |= VK_ACCESS_SHADER_READ_BIT;
            break;
        case BarrierType::Uniform:
            src_stage  |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            dst_stage  |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            src_access |= VK_ACCESS_SHADER_WRITE_BIT;
            dst_access |= VK_ACCESS_UNIFORM_READ_BIT;
            break;
        case BarrierType::VertexAttribArray:
        case BarrierType::ElementArray:
            src_stage  |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage  |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            src_access |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            dst_access |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
            break;
        case BarrierType::BufferUpdate:
        case BarrierType::TextureUpdate:
        case BarrierType::PixelBuffer:
            src_stage  |= VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            dst_stage  |= VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            src_access |= VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            dst_access |= VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
            break;
        case BarrierType::Framebuffer:
            src_stage  |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dst_stage  |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            src_access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dst_access |= VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            break;
        }
    }
    // Fall back to a conservative full barrier if no type matched.
    if (src_stage == 0) { src_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; }
    if (dst_stage == 0) { dst_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; }

    VkMemoryBarrier mb = {};
    mb.sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    mb.srcAccessMask = src_access;
    mb.dstAccessMask = dst_access;
    vkCmdPipelineBarrier(CurCmd(), src_stage, dst_stage, 0, 1, &mb, 0, nullptr, 0, nullptr);
}

// ===========================================================================
// Accessors used by RenderPass, FrameBuffers, Pipeline
// ===========================================================================

std::shared_ptr<PhysicalDevice> Context::GetPhysicalDevice() const
{
    return m_device.m_phy_dev;
}

std::shared_ptr<LogicalDevice> Context::GetLogicalDevice() const
{
    return m_device.m_logic_dev;
}

}
}
