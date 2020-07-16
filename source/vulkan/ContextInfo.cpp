#include "unirender/vulkan/ContextInfo.h"
#include "unirender/vulkan/Swapchain.h"
#include "unirender/vulkan/DepthBuffer.h"
#include "unirender/vulkan/CommandPool.h"
#include "unirender/vulkan/CommandBuffers.h"
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/DeviceInfo.h"
#include "unirender/vulkan/FrameBuffers.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/PipelineCache.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/PipelineLayout.h"
#include "unirender/vulkan/Pipeline.h"
#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/IndexBuffer.h"

#include <iostream>

namespace
{

struct Vertex {
    float posX, posY, posZ, posW;  // Position data
    float r, g, b, a;              // Color
};

#define XYZ1(_x_, _y_, _z_) (_x_), (_y_), (_z_), 1.f
#define UV(_u_, _v_) (_u_), (_v_)

static const Vertex g_vb_solid_face_colors_Data[] = {
    // red face
    {XYZ1(-1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    // green face
    {XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    // blue face
    {XYZ1(-1, 1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 1.f)},
    // yellow face
    {XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(1.f, 1.f, 0.f)},
    // magenta face
    {XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    // cyan face
    {XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
};

const char* vs = R"(
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (std140, binding = 0) uniform bufferVals {
    mat4 mvp;
} myBufferVals;
layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;
layout (location = 0) out vec4 outColor;
void main() {
   outColor = inColor;
   gl_Position = myBufferVals.mvp * pos;
}
)";

const char* fs = R"(
#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec4 color;
layout (location = 0) out vec4 outColor;
void main() {
    outColor = color;
}
)";

}

namespace ur
{
namespace vulkan
{

ContextInfo::ContextInfo(const DeviceInfo& dev_info, bool include_depth)
    : m_dev_info(dev_info)
    , m_include_depth(include_depth)
{
}

void ContextInfo::Init(int width, int height, void* hwnd)
{
    this->width = width;
    this->height = height;

	const bool use_texture = false;
	const bool include_vi = true;

    InitSwapchainExtension(hwnd);

    auto dev = m_dev_info.device;

	swapchain = std::make_shared<Swapchain>(dev);
	swapchain->Create(m_dev_info, *this);

    cmd_pool = std::make_shared<CommandPool>(dev);
    cmd_pool->Create();

    cmd_bufs = std::make_shared<CommandBuffers>(dev, cmd_pool);
    cmd_bufs->Create(swapchain->GetImageCount());

	depth_buf = std::make_shared<DepthBuffer>(dev);
	depth_buf->Create(m_dev_info, *this);

	uniform_buf = std::make_shared<UniformBuffer>(dev);
	uniform_buf->Create(m_dev_info, *this);

	desc_set_layout = std::make_shared<DescriptorSetLayout>(dev);
	desc_set_layout->Create(use_texture);

	pipeline_layout = std::make_shared<PipelineLayout>(dev);
	pipeline_layout->Create(*desc_set_layout);

	renderpass = std::make_shared<RenderPass>(dev);
	renderpass->Create(m_dev_info, *this, m_include_depth);

	frame_buffers = std::make_shared<FrameBuffers>(dev);
	frame_buffers->Create(m_dev_info, *this, m_include_depth);

	vert_buf = std::make_shared<vulkan::VertexBuffer>();
    vert_buf->Create(m_dev_info, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
        sizeof(g_vb_solid_face_colors_Data[0]), false);
    //uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);
    //vert_buf->Create(m_dev_info, vertexBuffer.data(), vertexBufferSize, sizeof(Vertex), false);

    //idx_buf = std::make_shared<vulkan::IndexBuffer>();
    //uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    //idx_buf->Create(m_dev_info, vertexBuffer.data(), indexBufferSize);

	program = std::make_shared<vulkan::ShaderProgram>(dev, vs, fs);

	desc_pool = std::make_shared<DescriptorPool>(dev);
	desc_pool->Create(use_texture);

	desc_set = std::make_shared<DescriptorSet>(dev);
	desc_set->Create(m_dev_info, *this, use_texture);

	pipeline_cache = std::make_shared<PipelineCache>(dev);
	pipeline_cache->Create();

	pipeline = std::make_shared<Pipeline>(dev);
	pipeline->Create(*this, m_include_depth, include_vi);
}

void ContextInfo::Resize(uint32_t width, uint32_t height)
{
    this->width = width;
    this->height = height;

    auto dev = m_dev_info.device;

    vkDeviceWaitIdle(m_dev_info.device);

    swapchain = std::make_shared<Swapchain>(dev);
    swapchain->Create(m_dev_info, *this);

    depth_buf = std::make_shared<DepthBuffer>(dev);
    depth_buf->Create(m_dev_info, *this);

    frame_buffers = std::make_shared<FrameBuffers>(dev);
    frame_buffers->Create(m_dev_info, *this, m_include_depth);

    cmd_bufs = std::make_shared<CommandBuffers>(dev, cmd_pool);
    cmd_bufs->Create(swapchain->GetImageCount());

    vkDeviceWaitIdle(m_dev_info.device);
}

void ContextInfo::InitSwapchainExtension(void* hwnd)
{
    /* DEPENDS on init_connection() and init_window() */

    VkResult res;

// Construct the surface description:
#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd = (HWND)hwnd;
    res = vkCreateWin32SurfaceKHR(m_dev_info.inst, &createInfo, NULL, &surface);
#else
    return;
#endif  // _WIN32
    assert(res == VK_SUCCESS);

    // Iterate over each queue to learn whether it supports presenting:
    VkBool32 *pSupportsPresent = (VkBool32 *)malloc(m_dev_info.queue_family_count * sizeof(VkBool32));
    for (uint32_t i = 0; i < m_dev_info.queue_family_count; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(m_dev_info.gpus[0], i, surface, &pSupportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    int curr_graphics_queue_family_index = UINT32_MAX;
    int curr_present_queue_family_index = UINT32_MAX;
    for (uint32_t i = 0; i < m_dev_info.queue_family_count; ++i) {
        if ((m_dev_info.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (curr_graphics_queue_family_index == UINT32_MAX) curr_graphics_queue_family_index = i;

            if (pSupportsPresent[i] == VK_TRUE) {
                curr_graphics_queue_family_index = i;
                curr_present_queue_family_index = i;
                break;
            }
        }
    }

    if (curr_present_queue_family_index == UINT32_MAX) {
        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (size_t i = 0; i < m_dev_info.queue_family_count; ++i)
            if (pSupportsPresent[i] == VK_TRUE) {
                curr_present_queue_family_index = i;
                break;
            }
    }
    free(pSupportsPresent);

    // Generate error if could not find queues that support graphics
    // and present
    if (curr_graphics_queue_family_index == UINT32_MAX || curr_present_queue_family_index == UINT32_MAX) {
        std::cout << "Could not find a queues for both graphics and present";
        exit(-1);
    }

    if (graphics_queue_family_index == UINT32_MAX) {
        graphics_queue_family_index = curr_graphics_queue_family_index;
    } else {
        assert(graphics_queue_family_index == curr_graphics_queue_family_index);
    }
    if (present_queue_family_index == UINT32_MAX) {
        present_queue_family_index = curr_present_queue_family_index;
    } else {
        assert(present_queue_family_index == curr_present_queue_family_index);
    }
    const_cast<DeviceInfo&>(m_dev_info).InitDeviceAndQueue(graphics_queue_family_index, present_queue_family_index);

    // Get the list of VkFormats that are supported:
    uint32_t formatCount;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_dev_info.gpus[0], surface, &formatCount, NULL);
    assert(res == VK_SUCCESS);
    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_dev_info.gpus[0], surface, &formatCount, surfFormats);
    assert(res == VK_SUCCESS);
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
        format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        assert(formatCount >= 1);
        format = surfFormats[0].format;
    }
    free(surfFormats);
}

}
}