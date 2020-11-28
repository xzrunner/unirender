#include "unirender/opengl/ImageFormat.h"

#include <assert.h>

namespace ur
{
namespace opengl
{

ImageFormat::ImageFormat(ur::TextureFormat fmt)
{
	switch(fmt)
    {
	case ur::TextureFormat::RGBA8:
		format = GL_RGBA8;		// rgba8
		break;
	case ur::TextureFormat::RGBA16F:
		format = GL_RGBA16F;	// rgba16f
		break;
	case ur::TextureFormat::RGBA32F:
		format = GL_RGBA32F;	// rgba32f
		break;
	case ur::TextureFormat::RG16F:
		format = GL_RG16F;		// rg16f
		break;
	case ur::TextureFormat::RED:
		format = GL_R8;			// r8
		break;
	case ur::TextureFormat::R16:
		format = GL_R16;		// r16
		break;
	default:
		assert(0);
	}
}

}
}