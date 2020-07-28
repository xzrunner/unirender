#include "unirender/vulkan/DescriptorSet.h"
#include "unirender/vulkan/DescriptorPool.h"
#include "unirender/vulkan/DescriptorSetLayout.h"
#include "unirender/vulkan/UniformBuffer.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/TypeConverter.h"
#include "unirender/vulkan/Texture.h"
#include "unirender/vulkan/ImageView.h"
#include "unirender/vulkan/TextureSampler.h"
#include "unirender/Device.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

DescriptorSet::DescriptorSet(const ur::Device& dev, const std::shared_ptr<LogicalDevice>& device, const ur::DescriptorPool& pool,
                             const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& _layouts,
                             const std::vector<ur::Descriptor>& descriptors)
	: m_device(device)
{
    std::vector<VkDescriptorSetLayout> layouts(_layouts.size());
    for (size_t i = 0, n = _layouts.size(); i < n; ++i) {
        layouts[i] = std::static_pointer_cast<vulkan::DescriptorSetLayout>(_layouts[i])->GetHandler();
    }

    VkDescriptorSetAllocateInfo alloc_info[1];
    alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info[0].pNext = NULL;
    alloc_info[0].descriptorPool = static_cast<const vulkan::DescriptorPool&>(pool).GetHandler();
    alloc_info[0].descriptorSetCount = layouts.size();
    alloc_info[0].pSetLayouts = layouts.data();

    VkResult res = vkAllocateDescriptorSets(m_device->GetHandler(), alloc_info, &m_handle);
    assert(res == VK_SUCCESS);

    std::vector<VkWriteDescriptorSet> vk_descriptors;
    vk_descriptors.reserve(descriptors.size());
    for (auto& desc : descriptors) 
    {
        VkWriteDescriptorSet write_desc_set = {};

        write_desc_set.dstSet = m_handle;

        write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_set.descriptorType = TypeConverter::To(desc.type);
        write_desc_set.dstBinding = vk_descriptors.size();
        if (desc.uniform) 
        {
            write_desc_set.pBufferInfo = &std::static_pointer_cast<UniformBuffer>(desc.uniform)->GetBufferInfo();
        } 
        else if (desc.texture) 
        {
            VkDescriptorImageInfo image_info = {};
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView = std::static_pointer_cast<Texture>(desc.texture)->GetImageView()->GetHandler();
            auto sampler = dev.GetTextureSampler(ur::Device::TextureSamplerType::LinearRepeat);
            image_info.sampler = std::static_pointer_cast<vulkan::TextureSampler>(sampler)->GetHandler();

            write_desc_set.pImageInfo = &image_info;
        }
        write_desc_set.descriptorCount = 1;

        vk_descriptors.push_back(write_desc_set);
    }
    vkUpdateDescriptorSets(m_device->GetHandler(), vk_descriptors.size(), vk_descriptors.data(), 0, NULL);
}

DescriptorSet::~DescriptorSet()
{
}

}
}