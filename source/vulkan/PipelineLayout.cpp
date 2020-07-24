#include "unirender/vulkan/PipelineLayout.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

PipelineLayout::PipelineLayout(const std::shared_ptr<LogicalDevice>& device,
	                           const std::vector<std::shared_ptr<DescriptorSetLayout>>& _layouts)
	: m_device(device)
{
	std::vector<VkDescriptorSetLayout> layouts(_layouts.size());
	for (size_t i = 0, n = _layouts.size(); i < n; ++i) {
		layouts[i] = _layouts[i]->GetHandler();
	}

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = layouts.size();
	pPipelineLayoutCreateInfo.pSetLayouts = layouts.data();

	VkResult res = vkCreatePipelineLayout(m_device->GetHandler(), &pPipelineLayoutCreateInfo, NULL, &m_handle);
	assert(res == VK_SUCCESS);
}

PipelineLayout::~PipelineLayout()
{
	vkDestroyPipelineLayout(m_device->GetHandler(), m_handle, NULL);
}

}
}