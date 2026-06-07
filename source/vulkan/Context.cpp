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

    vkDestroySemaphore(logic_dev, m_semaphores.present_complete, nullptr);
    vkDestroySemaphore(logic_dev, m_semaphores.render_complete, nullptr);
    vkDestroyFence(logic_dev, m_wait_fence, nullptr);
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

    // ---- 4. Command pool / buffer -----------------------------------------
    m_cmd_pool = std::make_shared<CommandPool>(m_device.m_logic_dev);
    if (!m_device.m_cmd_pool) {
        const_cast<Device&>(m_device).m_cmd_pool = m_cmd_pool;
    }
    m_cmd_buf = std::make_shared<CommandBuffer>(m_device.m_logic_dev, m_cmd_pool);

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

    // ---- 8. Synchronisation primitives ------------------------------------
    VkSemaphoreCreateInfo sem_ci = {};
    sem_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    auto vk_dev = m_device.m_logic_dev->GetHandler();
    VkResult res;
    res = vkCreateSemaphore(vk_dev, &sem_ci, nullptr, &m_semaphores.present_complete);
    assert(res == VK_SUCCESS);
    res = vkCreateSemaphore(vk_dev, &sem_ci, nullptr, &m_semaphores.render_complete);
    assert(res == VK_SUCCESS);

    VkFenceCreateInfo fence_ci = {};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signaled
    res = vkCreateFence(vk_dev, &fence_ci, nullptr, &m_wait_fence);
    assert(res == VK_SUCCESS);
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

    // Destroy in reverse order, then recreate
    m_frame_buffers.reset();
    m_renderpass.reset();
    m_depth_buf.reset();
    m_swapchain.reset();

    m_swapchain = std::make_shared<Swapchain>(
        m_device.m_logic_dev, *m_device.m_phy_dev, *m_surface, width, height);
    m_depth_buf = std::make_shared<DepthBuffer>(
        m_device.m_logic_dev, *m_device.m_phy_dev, width, height);
    m_renderpass    = std::make_shared<RenderPass>(*this, m_include_depth, /*clear=*/true);
    m_frame_buffers = std::make_shared<FrameBuffers>(*this, m_include_depth);

    m_cmd_buf = std::make_shared<CommandBuffer>(m_device.m_logic_dev, m_cmd_pool);
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

    auto vk_dev         = m_device.m_logic_dev->GetHandler();
    auto graphics_queue = m_device.m_logic_dev->GetGraphicsQueue();
    auto cmd_buf        = m_cmd_buf->GetHandler();

    // Each draw is its OWN command buffer, submitted and waited synchronously. This
    // lets the engine recreate vertex/index buffers between draws (it does, in
    // SpriteRenderer::Flush) without the GPU still referencing the freed ones, and
    // lets an offscreen FBO rebuild its render pass safely. The frame still
    // ACCUMULATES: screen passes use LOAD after the first clear, all targeting the
    // one swapchain image acquired this frame; present happens once in Flush().

    auto pool = std::static_pointer_cast<vulkan::DescriptorPool>(m_device.GetDescriptorPool());
    if (pool) {
        vkResetDescriptorPool(vk_dev, pool->GetHandler(), 0);
    }

    VkCommandBufferBeginInfo cb_begin = {};
    cb_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult res = vkBeginCommandBuffer(cmd_buf, &cb_begin);
    assert(res == VK_SUCCESS);

    BeginPass(fbo);            // lazy-acquires the image on the first screen pass
    RecordDraw(cmd_buf, ds, prim_type, offset, count);
    EndCurrentPass();

    res = vkEndCommandBuffer(cmd_buf);
    assert(res == VK_SUCCESS);

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si = {};
    si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &cmd_buf;
    // Only the first screen submit of the frame waits on the image-acquire semaphore.
    if (m_swapchain_acquired && m_first_screen_submit) {
        si.pWaitDstStageMask  = &wait_stage;
        si.pWaitSemaphores    = &m_semaphores.present_complete;
        si.waitSemaphoreCount = 1;
        m_first_screen_submit = false;
    }
    vkResetFences(vk_dev, 1, &m_wait_fence);
    res = vkQueueSubmit(graphics_queue, 1, &si, m_wait_fence);
    assert(res == VK_SUCCESS);
    res = vkWaitForFences(vk_dev, 1, &m_wait_fence, VK_TRUE, UINT64_MAX);
    assert(res == VK_SUCCESS);

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
    m_first_screen_submit = true;
    m_pass_open           = false;
    m_screen_cleared      = false;
    m_cur_pass_fbo        = nullptr;
}

void Context::EndCurrentPass()
{
    if (m_pass_open) {
        vkCmdEndRenderPass(m_cmd_buf->GetHandler());
        m_pass_open = false;
    }
}

void Context::BeginPass(const vulkan::Framebuffer* fbo)
{
    if (m_pass_open && m_cur_pass_fbo == fbo) {
        return; // already recording into the right target
    }
    EndCurrentPass();

    auto cmd_buf = m_cmd_buf->GetHandler();

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
        // Acquire a swapchain image the first time a screen pass is needed.
        if (!m_swapchain_acquired) {
            VkResult ar = vkAcquireNextImageKHR(
                m_device.m_logic_dev->GetHandler(), m_swapchain->GetHandler(), UINT64_MAX,
                m_semaphores.present_complete, VK_NULL_HANDLE, &m_current_buffer);
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
        auto pool = std::static_pointer_cast<vulkan::DescriptorPool>(m_device.GetDescriptorPool());
        // The pool is reset once per frame (BeginFrameIfNeeded); here we just
        // allocate a fresh set for this draw. Resetting mid-frame would free sets
        // still referenced by earlier draws recorded in this command buffer.
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
// WaitSync -- wait for previous frame to finish
// ===========================================================================

void Context::WaitSync()
{
    auto vk_dev = m_device.m_logic_dev->GetHandler();
    VkResult res;
    res = vkWaitForFences(vk_dev, 1, &m_wait_fence, VK_TRUE, UINT64_MAX);
    assert(res == VK_SUCCESS);
    res = vkResetFences(vk_dev, 1, &m_wait_fence);
    assert(res == VK_SUCCESS);
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
    // End-of-frame: present the swapchain image the frame's draws accumulated into.
    // Every draw already submitted + waited its fence, so the GPU is idle and the
    // image is in PRESENT_SRC -- present needs no wait semaphore. (Called from
    // main.cpp after the whole frame is drawn.)
    if (!m_frame_active) {
        return; // nothing was drawn this frame
    }

    // Diagnostic dump (TT_VK_DUMP=<frame>): the GPU is idle here (per-draw waits),
    // so the offscreen scene target can be copied out to inspect what rendered.
    ++m_frame_count;
    if (const char* d = std::getenv("TT_VK_DUMP")) {
        if (m_frame_count == atoi(d)) {
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
        auto graphics_queue = m_device.m_logic_dev->GetGraphicsQueue();
        // MoltenVK presents the drawable correctly only when present is ordered
        // after rendering by a semaphore. All render work is already done (each draw
        // waited its fence), so an empty submit just signals render_complete, then
        // present waits on it.
        VkSubmitInfo si = {};
        si.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores    = &m_semaphores.render_complete;
        VkResult sr = vkQueueSubmit(graphics_queue, 1, &si, VK_NULL_HANDLE);
        assert(sr == VK_SUCCESS);

        VkResult present = m_swapchain->QueuePresent(
            graphics_queue, m_current_buffer, m_semaphores.render_complete);
        if (present == VK_ERROR_OUT_OF_DATE_KHR || present == VK_SUBOPTIMAL_KHR) {
            Resize(m_width, m_height);
        }
    }

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
    // TODO: vkCmdPipelineBarrier for buffer/image memory barriers
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
