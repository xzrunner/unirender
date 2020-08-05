#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

UniformBuffer::UniformBuffer(const std::shared_ptr<LogicalDevice>& device,
                             const PhysicalDevice& phy_dev, const void* data, size_t size)
    : m_device(device)
    , m_buffer(device)
{
    auto vk_dev = device->GetHandler();

    m_buffer.Create(phy_dev.GetHandler(), size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    m_buffer.Upload(data, size);

    m_buffer_info.buffer = m_buffer.GetHandler();
    m_buffer_info.offset = 0;
    m_buffer_info.range = size;
}

void UniformBuffer::Update(const void* data, size_t size)
{
    m_buffer.Upload(data, size);
}

}
}