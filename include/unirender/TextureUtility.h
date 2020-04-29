#pragma once

#include "unirender/TextureFormat.h"

namespace ur
{

class TextureUtility
{
public:
    static int RequiredSizeInBytes(int width, int height,
        TextureFormat fmt, int row_alignment);

}; // TextureUtility

}