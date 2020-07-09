#pragma once

#include <memory>

namespace ur
{

class Device;

namespace vulkan
{

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

class ContextInfo
{
public:
    void Init(const ur::Device& device);

public:
    std::shared_ptr<CommandPool>   cmd_pool  = nullptr;
    std::shared_ptr<CommandBuffers> cmd_bufs = nullptr;

    std::shared_ptr<Swapchain>    swapchain = nullptr;
    std::shared_ptr<DepthBuffer>  depth_buf = nullptr;

    std::shared_ptr<UniformBuffer> uniform_buf = nullptr;

    std::shared_ptr<DescriptorSetLayout> desc_set_layout = nullptr;
    std::shared_ptr<PipelineLayout>      pipeline_layout = nullptr;

    std::shared_ptr<RenderPass>   renderpass = nullptr;
    std::shared_ptr<FrameBuffers> frame_buffers = nullptr;

    std::shared_ptr<VertexBuffer> vert_buf = nullptr;
    std::shared_ptr<ShaderProgram> program = nullptr;

    std::shared_ptr<DescriptorPool> desc_pool = nullptr;
    std::shared_ptr<DescriptorSet>  desc_set  = nullptr;

    std::shared_ptr<PipelineCache> pipeline_cache = nullptr;
    std::shared_ptr<Pipeline>      pipeline = nullptr;

}; // ContextInfo

}
}