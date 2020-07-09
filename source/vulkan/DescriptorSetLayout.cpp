#include "unirender/vulkan/DescriptorSetLayout.h"

#include <assert.h>

/* Number of descriptor sets needs to be the same at alloc,       */
/* pipeline layout creation, and descriptor set layout creation   */
#define NUM_DESCRIPTOR_SETS 1

namespace ur
{
namespace vulkan
{

DescriptorSetLayout::DescriptorSetLayout(VkDevice device)
	: m_device(device)
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	for (int i = 0; i < NUM_DESCRIPTOR_SETS; i++) {
		vkDestroyDescriptorSetLayout(m_device, m_handle[i], NULL);
	}
}

void DescriptorSetLayout::Create(bool use_texture, VkDescriptorSetLayoutCreateFlags descSetLayoutCreateFlags)
{
    VkDescriptorSetLayoutBinding layout_bindings[2];
    layout_bindings[0].binding = 0;
    layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[0].descriptorCount = 1;
    layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_bindings[0].pImmutableSamplers = NULL;

    if (use_texture)
	{
        layout_bindings[1].binding = 1;
        layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layout_bindings[1].descriptorCount = 1;
        layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layout_bindings[1].pImmutableSamplers = NULL;
    }

    /* Next take layout bindings and use them to create a descriptor set layout
     */
    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.flags = descSetLayoutCreateFlags;
    descriptor_layout.bindingCount = use_texture ? 2 : 1;
    descriptor_layout.pBindings = layout_bindings;

    VkResult res;

    m_handle.resize(NUM_DESCRIPTOR_SETS);
    res = vkCreateDescriptorSetLayout(m_device, &descriptor_layout, NULL, m_handle.data());
    assert(res == VK_SUCCESS);
}

}
}