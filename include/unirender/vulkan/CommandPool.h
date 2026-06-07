#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>


namespace ur
{
namespace vulkan
{

class LogicalDevice;

class CommandPool : noncopyable
{
public:
    // queue_family_index MUST match the queue the command buffers are submitted to
    // (the graphics family) -- VUID-vkQueueSubmit-pCommandBuffers-00074. It is taken
    // as a ctor arg because the VkCommandPool is created immediately here, so a later
    // SetQueueFamilyIndex() would be too late.
    CommandPool(const std::shared_ptr<LogicalDevice>& device, uint32_t queue_family_index = 0);
    ~CommandPool();

    void SetQueueFamilyIndex(uint32_t index) { m_queue_family_index = index; }

    auto GetHandler() const { return m_handle; }

private:
    std::shared_ptr<LogicalDevice> m_device = nullptr;

    VkCommandPool m_handle;

    uint32_t m_queue_family_index = 0;

}; // CommandPool

}
}