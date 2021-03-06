#pragma once

#include "unirender/DescriptorType.h"

#include <memory>

namespace ur
{

class UniformBuffer;
class Texture;

struct Descriptor
{
	Descriptor(DescriptorType type,
		const std::shared_ptr<ur::UniformBuffer>& uniform)
		: type(type), uniform(uniform) {}

	Descriptor(DescriptorType type,
		const std::shared_ptr<ur::Texture>& texture)
		: type(type), texture(texture) {}

	DescriptorType type;
	std::shared_ptr<ur::UniformBuffer> uniform = nullptr;
	std::shared_ptr<ur::Texture>       texture = nullptr;

};

}