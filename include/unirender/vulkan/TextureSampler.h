#pragma once

#include "unirender/TextureSampler.h"

#include <vulkan/vulkan.h>

#include <memory>

namespace ur
{
namespace vulkan
{

class LogicalDevice;

class TextureSampler : public ur::TextureSampler
{
public:
	TextureSampler(const std::shared_ptr<LogicalDevice>& device, TextureWrap wrap_u, 
		TextureWrap wrap_v, TextureWrap wrap_w, TextureMinificationFilter min_filter, 
		TextureMagnificationFilter mag_filter, float max_anistropy);
	virtual ~TextureSampler();

	auto GetHandler() const { return m_handle; }

	virtual void Bind(int tex_unit_idx) override {}
	static void UnBind(int tex_unit_idx) {}

private:
	std::shared_ptr<LogicalDevice> m_device = nullptr;

	VkSampler m_handle;

}; // TextureSampler

}
}