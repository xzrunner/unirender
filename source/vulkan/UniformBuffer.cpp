#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/Utility.h"
#include "unirender/vulkan/ContextInfo.h"

#include <glm/vec3.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace ur
{
namespace vulkan
{

UniformBuffer::UniformBuffer(VkDevice device)
    : m_device(device)
{
}

UniformBuffer::~UniformBuffer()
{
    vkDestroyBuffer(m_device, m_buf, NULL);
    vkFreeMemory(m_device, m_mem, NULL);
}

void UniformBuffer::Create(const ContextInfo& ctx_info)
{
    VkResult res;
    float fov = glm::radians(45.0f);
    if (ctx_info.width > ctx_info.height) {
        fov *= static_cast<float>(ctx_info.height) / static_cast<float>(ctx_info.width);
    }
    Projection = glm::perspective(fov, static_cast<float>(ctx_info.width) / static_cast<float>(ctx_info.height), 0.1f, 100.0f);
    View = glm::lookAt(glm::vec3(-5, 3, -10),  // Camera is at (-5,3,-10), in World Space
                            glm::vec3(0, 0, 0),     // and looks at the origin
                            glm::vec3(0, -1, 0)     // Head is up (set to 0,-1,0 to look upside-down)
    );
    Model = glm::mat4(1.0f);
    // Vulkan clip space has inverted Y and half Z.
    Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);

    MVP = Clip * Projection * View * Model;

    auto device = ctx_info.m_vk_ctx.GetDevice();

    /* VULKAN_KEY_START */
    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = sizeof(MVP);
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    res = vkCreateBuffer(device, &buf_info, NULL, &m_buf);
    assert(res == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device, m_buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = VulkanContext::FindMemoryType(
        ctx_info.m_vk_ctx.GetPhysicalDevice(), mem_reqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    res = vkAllocateMemory(device, &alloc_info, NULL, &(m_mem));
    assert(res == VK_SUCCESS);

    uint8_t *pData;
    res = vkMapMemory(device, m_mem, 0, mem_reqs.size, 0, (void **)&pData);
    assert(res == VK_SUCCESS);

    memcpy(pData, &MVP, sizeof(MVP));

    vkUnmapMemory(device, m_mem);

    res = vkBindBufferMemory(device, m_buf, m_mem, 0);
    assert(res == VK_SUCCESS);

    m_buffer_info.buffer = m_buf;
    m_buffer_info.offset = 0;
    m_buffer_info.range = sizeof(MVP);
}

}
}