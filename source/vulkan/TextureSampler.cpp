#include "unirender/vulkan/TextureSampler.h"
#include "unirender/vulkan/LogicalDevice.h"
#include "unirender/vulkan/TypeConverter.h"

#include <stdexcept>

namespace ur
{
namespace vulkan
{

TextureSampler::TextureSampler(const std::shared_ptr<LogicalDevice>& device, TextureWrap wrap_u, 
		                       TextureWrap wrap_v, TextureWrap wrap_w, TextureMinificationFilter min_filter, 
		                       TextureMagnificationFilter mag_filter, float max_anistropy)
	: ur::TextureSampler(min_filter, mag_filter, wrap_u, wrap_v, max_anistropy)
    , m_device(device)
{
	VkSamplerCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    ci.addressModeU  = TypeConverter::To(wrap_u);
    ci.addressModeV  = TypeConverter::To(wrap_v);
    ci.addressModeW  = TypeConverter::To(wrap_w);
    ci.borderColor   = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    ci.minFilter     = TypeConverter::To(min_filter);
    ci.magFilter     = TypeConverter::To(mag_filter);
    ci.maxAnisotropy = max_anistropy;
    if (vkCreateSampler(m_device->GetHandler(), &ci, nullptr, &m_handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create sampler!");
    }
}

TextureSampler::~TextureSampler()
{
	vkDestroySampler(m_device->GetHandler(), m_handle, nullptr);
}

}
}