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
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/IndexBuffer.h"
#include "unirender/vulkan/Surface.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Instance.h"
#include "unirender/vulkan/Texture.h"
#include "unirender/Adaptor.h"
#include "unirender/DrawState.h"
#include "unirender/VertexArray.h"
#include "unirender/VertexArraySizes.h"

#include <vulkan/vulkan.h>

#include <iostream>
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

    if (static_cast<uint32_t>(m_clear_flag) & static_cast<uint32_t>(ClearBuffers::ColorBuffer)) {
        m_clear_color = clear_state.color;
    }
    if (static_cast<uint32_t>(m_clear_flag) & static_cast<uint32_t>(ClearBuffers::DepthBuffer)) {
        m_clear_depth = clear_state.depth;
    }
    if (static_cast<uint32_t>(m_clear_flag) & static_cast<uint32_t>(ClearBuffers::StencilBuffer)) {
        m_clear_stencil = clear_state.stencil;
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
    if (count == 0) return;

    // ---- 1. Wait for previous frame's fence ----
    WaitSync();

    // ---- 2. Acquire next swapchain image ----
    auto vk_dev = m_device.m_logic_dev->GetHandler();
    VkResult res = vkAcquireNextImageKHR(
        vk_dev, m_swapchain->GetHandler(), UINT64_MAX,
        m_semaphores.present_complete, VK_NULL_HANDLE, &m_current_buffer);

    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        Resize(m_width, m_height);
        return;
    }
    assert(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR);

    // ---- 3. Build command buffer ----
    BuildCommandBuffer(ds, prim_type, offset, count);

    // ---- 4. Submit command buffer ----
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = {};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask    = &wait_stage;
    submit_info.pWaitSemaphores      = &m_semaphores.present_complete;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pSignalSemaphores    = &m_semaphores.render_complete;
    submit_info.signalSemaphoreCount = 1;
    auto cmd_buf = m_cmd_buf->GetHandler();
    submit_info.pCommandBuffers      = &cmd_buf;
    submit_info.commandBufferCount   = 1;

    auto graphics_queue = m_device.m_logic_dev->GetGraphicsQueue();
    res = vkQueueSubmit(graphics_queue, 1, &submit_info, m_wait_fence);
    assert(res == VK_SUCCESS);

    // ---- 5. Present ----
    VkResult present = m_swapchain->QueuePresent(
        graphics_queue, m_current_buffer, m_semaphores.render_complete);
    if (present == VK_ERROR_OUT_OF_DATE_KHR || present == VK_SUBOPTIMAL_KHR) {
        Resize(m_width, m_height);
    } else {
        assert(present == VK_SUCCESS);
    }
}

// ===========================================================================
// BuildCommandBuffer -- records one frame
// ===========================================================================

void Context::BuildCommandBuffer(const DrawState& ds, PrimitiveType prim_type,
                                 int offset, int count)
{
    VkResult res;
    auto cmd_buf = m_cmd_buf->GetHandler();

    VkCommandBufferBeginInfo cb_begin = {};
    cb_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    res = vkBeginCommandBuffer(cmd_buf, &cb_begin);
    assert(res == VK_SUCCESS);

    // ---- Begin render pass ----
    std::vector<VkClearValue> clear_values;
    clear_values.resize(m_include_depth ? 2 : 1);
    clear_values[0].color = {{
        m_clear_color.r / 255.0f,
        m_clear_color.g / 255.0f,
        m_clear_color.b / 255.0f,
        m_clear_color.a / 255.0f
    }};
    if (m_include_depth) {
        clear_values[1].depthStencil = {
            static_cast<float>(m_clear_depth),
            static_cast<uint32_t>(m_clear_stencil)
        };
    }

    VkRenderPassBeginInfo rp_begin = {};
    rp_begin.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass        = m_renderpass->GetHandler();
    rp_begin.framebuffer       = m_frame_buffers->GetHandler(m_current_buffer);
    rp_begin.renderArea.offset = {0, 0};
    rp_begin.renderArea.extent = { static_cast<uint32_t>(m_width),
                                   static_cast<uint32_t>(m_height) };
    rp_begin.clearValueCount   = static_cast<uint32_t>(clear_values.size());
    rp_begin.pClearValues      = clear_values.data();

    vkCmdBeginRenderPass(cmd_buf, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    // ---- Viewport & scissor (dynamic state) ----
    VkViewport viewport = {};
    viewport.x        = static_cast<float>(m_viewport.x);
    viewport.y        = static_cast<float>(m_viewport.y);
    viewport.width    = (m_viewport.w > 0) ? static_cast<float>(m_viewport.w)
                                           : static_cast<float>(m_width);
    viewport.height   = (m_viewport.h > 0) ? static_cast<float>(m_viewport.h)
                                           : static_cast<float>(m_height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buf, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.extent.width  = static_cast<uint32_t>(m_width);
    scissor.extent.height = static_cast<uint32_t>(m_height);
    vkCmdSetScissor(cmd_buf, 0, 1, &scissor);

    // ---- Bind descriptor sets ----
    if (ds.desc_set) {
        std::vector<VkDescriptorSet> desc_sets;
        desc_sets.push_back(
            std::static_pointer_cast<vulkan::DescriptorSet>(ds.desc_set)->GetHandler());
        vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
            std::static_pointer_cast<vulkan::PipelineLayout>(ds.pipeline_layout)->GetHandler(),
            0, static_cast<uint32_t>(desc_sets.size()), desc_sets.data(), 0, nullptr);
    }

    // ---- Bind pipeline ----
    if (ds.pipeline) {
        vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
            std::static_pointer_cast<vulkan::Pipeline>(ds.pipeline)->GetHandler());
    }

    // ---- Bind vertex / index buffers, draw ----
    if (ds.vertex_array)
    {
        auto vb = ds.vertex_array->GetVertexBuffer();
        auto ib = ds.vertex_array->GetIndexBuffer();

        VkDeviceSize vb_offsets[1] = {0};
        if (vb) {
            auto vk_vb = std::static_pointer_cast<vulkan::VertexBuffer>(vb)->GetBuffer();
            if (vk_vb != VK_NULL_HANDLE) {
                vkCmdBindVertexBuffers(cmd_buf, 0, 1, &vk_vb, vb_offsets);
            }
        }

        if (ib) {
            auto vk_ib = std::static_pointer_cast<vulkan::IndexBuffer>(ib)->GetBuffer();
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

    vkCmdEndRenderPass(cmd_buf);

    res = vkEndCommandBuffer(cmd_buf);
    assert(res == VK_SUCCESS);
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

void Context::Flush()
{
    vkDeviceWaitIdle(m_device.m_logic_dev->GetHandler());
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
