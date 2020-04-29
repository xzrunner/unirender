#pragma once

#include "unirender/TextureFormat.h"

namespace ur
{

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

}; // Texture

}