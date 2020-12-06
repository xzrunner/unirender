#pragma once

#include <memory>

namespace ur
{

class Texture;

class ImageUnit
{
public:
	~ImageUnit() {}

	virtual void SetTexture(const std::shared_ptr<Texture>& texture) = 0;

}; // ImageUnit

}