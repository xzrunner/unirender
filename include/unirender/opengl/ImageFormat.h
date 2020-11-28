#pragma once

#include "unirender/TextureFormat.h"
#include "unirender/opengl/opengl.h"

namespace ur
{
namespace opengl
{

struct ImageFormat
{
	ImageFormat(ur::TextureFormat fmt);

	GLenum format;

}; // ImageFormat

}
}