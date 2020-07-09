#include "unirender/vulkan/PipelineLayout.h"
#include "unirender/vulkan/DescriptorSetLayout.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

PipelineLayout::PipelineLayout(VkDevice device)
	: m_device(device)
{
}

PipelineLayout::~PipelineLayout()
{
	vkDestroyPipelineLayout(m_device, m_handle, NULL);
}

void PipelineLayout::Create(const DescriptorSetLayout& desc_set_layout)
{
	auto& desc_set_layouts = desc_set_layout.GetHandler();

	/* Now use the descriptor layout to create a pipeline layout */
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = desc_set_layouts.size();
	pPipelineLayoutCreateInfo.pSetLayouts = desc_set_layouts.data();

	VkResult res = vkCreatePipelineLayout(m_device, &pPipelineLayoutCreateInfo, NULL, &m_handle);
	assert(res == VK_SUCCESS);
}

}
}