#pragma once

#include "unirender/TextureTarget.h"
#include "unirender/TextureFormat.h"

namespace ur
{

struct TextureDescription
{
    TextureTarget target = TextureTarget::Texture2D;

    int width  = 0;
    int height = 0;
    int depth  = 0;

    TextureFormat format      = TextureFormat::RGBA8;
    bool          gen_mipmaps = false;

}; // TextureDescription

}