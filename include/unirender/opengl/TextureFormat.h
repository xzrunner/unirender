#pragma once

#include "unirender/TextureFormat.h"
#include "unirender/opengl/opengl.h"

namespace ur
{
namespace opengl
{

struct TextureFormat
{
    TextureFormat(ur::TextureFormat fmt, bool gamma_correction = false);

    GLint  internal_format;
    GLenum pixel_format;
    GLenum pixel_type;

    bool compressed = false;

}; // TextureFormat

}
}