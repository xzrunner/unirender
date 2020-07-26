#include "unirender/vulkan/PipelineLayout.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

PipelineLayout::PipelineLayout(const std::shared_ptr<LogicalDevice>& device,
	                           const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& _layouts)
	: m_device(device)
{
	std::vector<VkDescriptorSetLayout> vk_layouts(_layouts.size());
	for (size_t i = 0, n = _layouts.size(); i < n; ++i) {
		vk_layouts[i] = std::static_pointer_cast<DescriptorSetLayout>(_layouts[i])->GetHandler();
	}

	VkPipelineLayoutCreateInfo pipeline_layout_ci = {};
	pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_ci.pNext = NULL;
	pipeline_layout_ci.pushConstantRangeCount = 0;
	pipeline_layout_ci.pPushConstantRanges = NULL;
	pipeline_layout_ci.setLayoutCount = vk_layouts.size();
	pipeline_layout_ci.pSetLayouts = vk_layouts.data();

	VkResult res = vkCreatePipelineLayout(m_device->GetHandler(), &pipeline_layout_ci, NULL, &m_handle);
	assert(res == VK_SUCCESS);
}

PipelineLayout::~PipelineLayout()
{
	vkDestroyPipelineLayout(m_device->GetHandler(), m_handle, NULL);
}

}
}