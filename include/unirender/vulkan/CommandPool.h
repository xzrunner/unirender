#pragma once

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class CommandPool
{
public:
    CommandPool(VkDevice device);
    ~CommandPool();

    void Create();

    void SetQueueFamilyIndex(uint32_t index) { m_queue_family_index = index; }

    auto GetHandler() const { return m_handle; }

private:
    VkDevice m_device = VK_NULL_HANDLE;

    VkCommandPool m_handle;

    uint32_t m_queue_family_index = 0;

}; // CommandPool

}
}