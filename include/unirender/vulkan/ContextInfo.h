#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace ur
{
namespace vulkan
{

class DeviceInfo;
class CommandPool;
class CommandBuffers;
class Swapchain;
class DepthBuffer;
class UniformBuffer;
class RenderPass;
class FrameBuffers;
class DescriptorPool;
class DescriptorSet;
class PipelineCache;
class DescriptorSetLayout;
class PipelineLayout;
class Pipeline;
class ShaderProgram;
class VertexBuffer;
class IndexBuffer;

class ContextInfo
{
public:
    ContextInfo(const DeviceInfo& dev_info, bool include_depth);

    void Init(int width, int height, void* hwnd);

    void Resize(uint32_t width, uint32_t height);

private:
    void InitSwapchainExtension(void* hwnd);

public:
    int width = 0, height = 0;



    VkSurfaceKHR surface = VK_NULL_HANDLE;

    uint32_t graphics_queue_family_index = UINT32_MAX;
    uint32_t present_queue_family_index = UINT32_MAX;

    uint32_t current_buffer = 0;

    VkFormat format;

    std::shared_ptr<CommandPool>   cmd_pool  = nullptr;
    std::shared_ptr<CommandBuffers> cmd_bufs = nullptr;

    std::shared_ptr<Swapchain>    swapchain = nullptr;
    std::shared_ptr<DepthBuffer>  depth_buf = nullptr;

    std::shared_ptr<UniformBuffer> uniform_buf = nullptr;

    std::shared_ptr<DescriptorSetLayout> desc_set_layout = nullptr;
    std::shared_ptr<PipelineLayout>      pipeline_layout = nullptr;

    std::shared_ptr<RenderPass>   renderpass = nullptr;
    std::shared_ptr<FrameBuffers> frame_buffers = nullptr;

    std::shared_ptr<VertexBuffer>  vert_buf = nullptr;
    std::shared_ptr<IndexBuffer>   idx_buf  = nullptr;
    std::shared_ptr<ShaderProgram> program  = nullptr;

    std::shared_ptr<DescriptorPool> desc_pool = nullptr;
    std::shared_ptr<DescriptorSet>  desc_set  = nullptr;

    std::shared_ptr<PipelineCache> pipeline_cache = nullptr;
    std::shared_ptr<Pipeline>      pipeline = nullptr;

private:
    const DeviceInfo& m_dev_info;

    bool m_include_depth = false;

}; // ContextInfo

}
}