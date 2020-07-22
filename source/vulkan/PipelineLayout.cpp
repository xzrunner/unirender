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

void PipelineLayout::Create()
{
	std::vector<VkDescriptorSetLayout> layouts(m_layouts.size());
	for (size_t i = 0, n = m_layouts.size(); i < n; ++i) {
		layouts[i] = m_layouts[i]->GetHandler();
	}

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = layouts.size();
	pPipelineLayoutCreateInfo.pSetLayouts = layouts.data();

	VkResult res = vkCreatePipelineLayout(m_device, &pPipelineLayoutCreateInfo, NULL, &m_handle);
	assert(res == VK_SUCCESS);
}

}
}