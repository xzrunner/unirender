#pragma once

#include <vulkan/vulkan.h>

#include <memory>

#include <boost/noncopyable.hpp>

namespace ur
{
namespace vulkan
{

class LogicalDevice;

class CommandPool : boost::noncopyable
{
public:
    CommandPool(const std::shared_ptr<LogicalDevice>& device);
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