#pragma once

#include "unirender/TextureFormat.h"
#include "unirender/AccessType.h"

#include <memory>

namespace ur
{

class TextureSampler;

class Texture
{
public:
    virtual ~Texture() {}

    virtual int GetTexID() const = 0;

    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual int GetDepth() const = 0;

    virtual TextureFormat GetFormat() const = 0;

    virtual void Bind() const = 0;

    virtual void Upload(const void* pixels, int x, int y, int w, int h,
        int miplevel = 0, int row_alignment = 4) = 0;

    virtual void ApplySampler(const std::shared_ptr<TextureSampler>& sampler) = 0;

    virtual void BindToImage(uint32_t unit, AccessType access) const = 0;

}; // Texture

}