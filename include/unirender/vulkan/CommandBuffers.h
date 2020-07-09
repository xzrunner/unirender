#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace ur
{
namespace vulkan
{

class CommandPool;

class CommandBuffers
{
public:
    CommandBuffers(VkDevice device, const std::shared_ptr<CommandPool>& pool);
    ~CommandBuffers();

    void Create(int count);

    auto& GetHandler() const { return m_handle; }

private:
    VkDevice m_device;

    std::vector<VkCommandBuffer> m_handle;

    std::shared_ptr<CommandPool> m_pool = nullptr;

}; // CommandBuffers

}
}