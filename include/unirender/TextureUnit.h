#pragma once

#include <memory>

namespace ur
{

class Texture;
class TextureSampler;

class TextureUnit
{
public:
    virtual ~TextureUnit() {}

    virtual void SetTexture(const std::shared_ptr<Texture>& texture) = 0;
    virtual void SetSampler(const std::shared_ptr<TextureSampler>& sampler) = 0;

}; // TextureUnit

}