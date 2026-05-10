#pragma once

#include "unirender/noncopyable.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace ur
{
namespace vulkan
{

class Context;
class LogicalDevice;

class FrameBuffers : noncopyable
{
public:
    FrameBuffers(const Context& ctx, bool include_depth);
    ~FrameBuffers();

    // FIX: add indexed accessor used by Context::BuildCommandBuffer
    VkFramebuffer GetHandler(uint32_t index) const {
        if (index < m_frame_buffers.size()) {
            return m_frame_buffers[index];
        }
        return VK_NULL_HANDLE;
    }

    size_t GetCount() const { return m_frame_buffers.size(); }

private:
    std::shared_ptr<LogicalDevice> m_device = nullptr;

    std::vector<VkFramebuffer> m_frame_buffers;

}; // FrameBuffers

}
}
