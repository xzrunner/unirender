#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;
class CommandPool;

class CommandBuffers : boost::noncopyable
{
public:
    CommandBuffers(const std::shared_ptr<LogicalDevice>& device, 
        const std::shared_ptr<CommandPool>& pool, int count);
    ~CommandBuffers();

    auto& GetHandler() const { return m_handle; }

private:
    std::shared_ptr<LogicalDevice> m_device = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> m_handle;

    std::shared_ptr<CommandPool> m_pool = nullptr;

}; // CommandBuffers

}
}