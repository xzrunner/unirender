#pragma once

#include <memory>

namespace ur
{

class Texture;

class TextureBuffer
{
public:
	virtual ~TextureBuffer() {}

	virtual void ReadFromMemory(const void* data, int size, int offset) = 0;

	virtual void Bind() const = 0;
	virtual void UnBind() const = 0;

	virtual std::shared_ptr<Texture> GetTexture() const = 0;

}; // TextureBuffer

}