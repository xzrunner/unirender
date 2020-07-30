#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/PhysicalDevice.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/Utility.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

UniformBuffer::UniformBuffer(const std::shared_ptr<LogicalDevice>& device,
                             const PhysicalDevice& phy_dev, const void* data, size_t size)
    : m_device(device)
{
    auto vk_dev = device->GetHandler();

    Utility::CreateBuffer(vk_dev, phy_dev.GetHandler(), size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buf, m_mem);

    Update(data, size);

    m_buffer_info.buffer = m_buf;
    m_buffer_info.offset = 0;
    m_buffer_info.range = size;
}

UniformBuffer::~UniformBuffer()
{
    vkDestroyBuffer(m_device->GetHandler(), m_buf, NULL);
    vkFreeMemory(m_device->GetHandler(), m_mem, NULL);
}

void UniformBuffer::Update(const void* data, size_t size)
{
    auto vk_dev = m_device->GetHandler();

    uint8_t* dst;
    VkResult res = vkMapMemory(vk_dev, m_mem, 0, size, 0, (void**)&dst);
    assert(res == VK_SUCCESS);

    memcpy(dst, data, size);

    vkUnmapMemory(vk_dev, m_mem);
}

}
}