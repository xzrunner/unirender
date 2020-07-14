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

//struct Vertex {
//    float position[3];
//    float color[3];
//};

//const std::vector<Vertex> vertexBuffer =
//{
//	{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
//	{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
//	{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
//};
//
//std::vector<uint32_t> indexBuffer = { 0, 1, 2 };


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

//const char* vs = R"(
//#version 450
//
//layout (location = 0) in vec3 inPos;
//layout (location = 1) in vec3 inColor;
//
//layout (binding = 0) uniform UBO 
//{
//	mat4 projectionMatrix;
//	mat4 modelMatrix;
//	mat4 viewMatrix;
//} ubo;
//
//layout (location = 0) out vec3 outColor;
//
//out gl_PerVertex 
//{
//    vec4 gl_Position;   
//};
//
//
//void main() 
//{
//	outColor = inColor;
//	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
//}
//)";
//
//const char* fs = R"(
//#version 450
//
//layout (location = 0) in vec3 inColor;
//
//layout (location = 0) out vec4 outFragColor;
//
//void main() 
//{
//  outFragColor = vec4(inColor, 1.0);
//}
//)";

static const uint32_t __draw_cube_vert[287] = {
    0x07230203, 0x00010000, 0x00080008, 0x00000020,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0009000f, 0x00000000, 0x00000004, 0x6e69616d,
    0x00000000, 0x00000009, 0x0000000b, 0x00000012,
    0x0000001c, 0x00030003, 0x00000002, 0x00000190,
    0x00090004, 0x415f4c47, 0x735f4252, 0x72617065,
    0x5f657461, 0x64616873, 0x6f5f7265, 0x63656a62,
    0x00007374, 0x00090004, 0x415f4c47, 0x735f4252,
    0x69646168, 0x6c5f676e, 0x75676e61, 0x5f656761,
    0x70303234, 0x006b6361, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00050005, 0x00000009,
    0x4374756f, 0x726f6c6f, 0x00000000, 0x00040005,
    0x0000000b, 0x6f436e69, 0x00726f6c, 0x00060005,
    0x00000010, 0x505f6c67, 0x65567265, 0x78657472,
    0x00000000, 0x00060006, 0x00000010, 0x00000000,
    0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006,
    0x00000010, 0x00000001, 0x505f6c67, 0x746e696f,
    0x657a6953, 0x00000000, 0x00070006, 0x00000010,
    0x00000002, 0x435f6c67, 0x4470696c, 0x61747369,
    0x0065636e, 0x00030005, 0x00000012, 0x00000000,
    0x00050005, 0x00000016, 0x66667562, 0x61567265,
    0x0000736c, 0x00040006, 0x00000016, 0x00000000,
    0x0070766d, 0x00060005, 0x00000018, 0x7542796d,
    0x72656666, 0x736c6156, 0x00000000, 0x00030005,
    0x0000001c, 0x00736f70, 0x00040047, 0x00000009,
    0x0000001e, 0x00000000, 0x00040047, 0x0000000b,
    0x0000001e, 0x00000001, 0x00050048, 0x00000010,
    0x00000000, 0x0000000b, 0x00000000, 0x00050048,
    0x00000010, 0x00000001, 0x0000000b, 0x00000001,
    0x00050048, 0x00000010, 0x00000002, 0x0000000b,
    0x00000003, 0x00030047, 0x00000010, 0x00000002,
    0x00040048, 0x00000016, 0x00000000, 0x00000005,
    0x00050048, 0x00000016, 0x00000000, 0x00000023,
    0x00000000, 0x00050048, 0x00000016, 0x00000000,
    0x00000007, 0x00000010, 0x00030047, 0x00000016,
    0x00000002, 0x00040047, 0x00000018, 0x00000022,
    0x00000000, 0x00040047, 0x00000018, 0x00000021,
    0x00000000, 0x00040047, 0x0000001c, 0x0000001e,
    0x00000000, 0x00020013, 0x00000002, 0x00030021,
    0x00000003, 0x00000002, 0x00030016, 0x00000006,
    0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000004, 0x00040020, 0x00000008, 0x00000003,
    0x00000007, 0x0004003b, 0x00000008, 0x00000009,
    0x00000003, 0x00040020, 0x0000000a, 0x00000001,
    0x00000007, 0x0004003b, 0x0000000a, 0x0000000b,
    0x00000001, 0x00040015, 0x0000000d, 0x00000020,
    0x00000000, 0x0004002b, 0x0000000d, 0x0000000e,
    0x00000001, 0x0004001c, 0x0000000f, 0x00000006,
    0x0000000e, 0x0005001e, 0x00000010, 0x00000007,
    0x00000006, 0x0000000f, 0x00040020, 0x00000011,
    0x00000003, 0x00000010, 0x0004003b, 0x00000011,
    0x00000012, 0x00000003, 0x00040015, 0x00000013,
    0x00000020, 0x00000001, 0x0004002b, 0x00000013,
    0x00000014, 0x00000000, 0x00040018, 0x00000015,
    0x00000007, 0x00000004, 0x0003001e, 0x00000016,
    0x00000015, 0x00040020, 0x00000017, 0x00000002,
    0x00000016, 0x0004003b, 0x00000017, 0x00000018,
    0x00000002, 0x00040020, 0x00000019, 0x00000002,
    0x00000015, 0x0004003b, 0x0000000a, 0x0000001c,
    0x00000001, 0x00050036, 0x00000002, 0x00000004,
    0x00000000, 0x00000003, 0x000200f8, 0x00000005,
    0x0004003d, 0x00000007, 0x0000000c, 0x0000000b,
    0x0003003e, 0x00000009, 0x0000000c, 0x00050041,
    0x00000019, 0x0000001a, 0x00000018, 0x00000014,
    0x0004003d, 0x00000015, 0x0000001b, 0x0000001a,
    0x0004003d, 0x00000007, 0x0000001d, 0x0000001c,
    0x00050091, 0x00000007, 0x0000001e, 0x0000001b,
    0x0000001d, 0x00050041, 0x00000008, 0x0000001f,
    0x00000012, 0x00000014, 0x0003003e, 0x0000001f,
    0x0000001e, 0x000100fd, 0x00010038,
};

static const uint32_t __draw_cube_frag[112] = {
    0x07230203, 0x00010000, 0x00080008, 0x0000000d,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001,
    0x0007000f, 0x00000004, 0x00000004, 0x6e69616d,
    0x00000000, 0x00000009, 0x0000000b, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002,
    0x00000190, 0x00090004, 0x415f4c47, 0x735f4252,
    0x72617065, 0x5f657461, 0x64616873, 0x6f5f7265,
    0x63656a62, 0x00007374, 0x00090004, 0x415f4c47,
    0x735f4252, 0x69646168, 0x6c5f676e, 0x75676e61,
    0x5f656761, 0x70303234, 0x006b6361, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00050005,
    0x00000009, 0x4374756f, 0x726f6c6f, 0x00000000,
    0x00040005, 0x0000000b, 0x6f6c6f63, 0x00000072,
    0x00040047, 0x00000009, 0x0000001e, 0x00000000,
    0x00040047, 0x0000000b, 0x0000001e, 0x00000000,
    0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020,
    0x00040017, 0x00000007, 0x00000006, 0x00000004,
    0x00040020, 0x00000008, 0x00000003, 0x00000007,
    0x0004003b, 0x00000008, 0x00000009, 0x00000003,
    0x00040020, 0x0000000a, 0x00000001, 0x00000007,
    0x0004003b, 0x0000000a, 0x0000000b, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000,
    0x00000003, 0x000200f8, 0x00000005, 0x0004003d,
    0x00000007, 0x0000000c, 0x0000000b, 0x0003003e,
    0x00000009, 0x0000000c, 0x000100fd, 0x00010038,
};

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

	program = std::make_shared<vulkan::ShaderProgram>(dev, 
        __draw_cube_vert, sizeof(__draw_cube_vert) / sizeof(__draw_cube_vert[0]), 
        __draw_cube_frag, sizeof(__draw_cube_frag) / sizeof(__draw_cube_frag[0])
    );

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