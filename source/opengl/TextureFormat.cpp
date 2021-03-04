#include "unirender/opengl/TextureFormat.h"

#include <assert.h>

namespace ur
{
namespace opengl
{

TextureFormat::TextureFormat(ur::TextureFormat fmt, bool gamma_correction)
{
	if (gamma_correction) {
		int zz = 0;
	}

    compressed = false;
	switch(fmt)
    {
	case ur::TextureFormat::RGBA8:
		internal_format = gamma_correction ? GL_SRGB8_ALPHA8 : GL_RGBA;
		pixel_format = GL_RGBA;
		pixel_type = GL_UNSIGNED_BYTE;
		break;
	case ur::TextureFormat::RGB:
		internal_format = gamma_correction ? GL_SRGB8 : GL_RGB;
		pixel_format = GL_RGB;
		pixel_type = GL_UNSIGNED_BYTE;
		break;
	case ur::TextureFormat::RGBA4:
		internal_format = pixel_format = GL_RGBA;
		pixel_type = GL_UNSIGNED_SHORT_4_4_4_4;
		break;
	case ur::TextureFormat::RGB565:
		internal_format = pixel_format = GL_RGB;
		pixel_type = GL_UNSIGNED_SHORT_5_6_5;
		break;
	case ur::TextureFormat::BGRA_EXT:
		internal_format = GL_RGBA;
		pixel_format = GL_BGRA;
		pixel_type = GL_UNSIGNED_BYTE;
		break;
	case ur::TextureFormat::BGR_EXT:
		internal_format = GL_RGB;
		pixel_format = GL_BGR;
		pixel_type = GL_UNSIGNED_BYTE;
		break;
    case ur::TextureFormat::RGBA16F:
        internal_format = GL_RGBA16F;
        pixel_format = GL_RGBA;
        pixel_type = GL_FLOAT;
        break;
	case ur::TextureFormat::RGBA32F:
		internal_format = GL_RGBA32F;
		pixel_format = GL_RGBA;
		pixel_type = GL_FLOAT;
		break;
    case ur::TextureFormat::RGB16F:
        internal_format = GL_RGB16F;
        pixel_format = GL_RGB;
        pixel_type = GL_FLOAT;
        break;
    case ur::TextureFormat::RGB32F:
        internal_format = GL_RGB32F;
        pixel_format = GL_RGB;
        pixel_type = GL_FLOAT;
        break;
    case ur::TextureFormat::RG16F:
        internal_format = GL_RG16F;
        pixel_format = GL_RG;
        pixel_type = GL_FLOAT;
        break;
	case ur::TextureFormat::A8 :
        internal_format = pixel_format = GL_ALPHA;
        pixel_type = GL_UNSIGNED_BYTE;
        break;
    case ur::TextureFormat::RED:
        internal_format = GL_R8;
        pixel_format = GL_RED;
        pixel_type = GL_UNSIGNED_BYTE;
        break;
    case ur::TextureFormat::R16:
        internal_format = GL_R16_SNORM;
        pixel_format = GL_RED;
        pixel_type = GL_SHORT;
        break;
	case ur::TextureFormat::DEPTH :
	#if OPENGLES == 3 || OPENGLES == 0
		internal_format = GL_DEPTH_COMPONENT;
		pixel_format = GL_DEPTH_COMPONENT;
	#else
		internal_format = pixel_format = GL_ALPHA;
	#endif
		pixel_type = GL_FLOAT;
		break;
	case ur::TextureFormat::RGB32I:
		internal_format = GL_RGB32I;
		pixel_format = GL_RGB_INTEGER;
		pixel_type = GL_INT;
		break;
#ifdef GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
	case ur::TextureFormat::PVR2 :
		internal_format = pixel_format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		compressed = true;
		break;
#endif
#ifdef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
	case ur::TextureFormat::PVR4 :
		internal_format = pixel_format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		compressed = true;
		break;
#endif
#ifdef GL_ETC1_RGB8_OES
	case ur::TextureFormat::ETC1 :
		internal_format = pixel_format = GL_ETC1_RGB8_OES;
		compressed = true;
		break;
#endif // GL_ETC1_RGB8_OES
	case ur::TextureFormat::ETC2:
		internal_format = gamma_correction ? GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC : GL_COMPRESSED_RGBA8_ETC2_EAC;
		pixel_format = GL_COMPRESSED_RGBA8_ETC2_EAC;
		compressed = true;
		break;
	case ur::TextureFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:
		internal_format = pixel_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		compressed = true;
		break;
	case ur::TextureFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:
		internal_format = pixel_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		compressed = true;
		break;
	case ur::TextureFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:
		internal_format = pixel_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		compressed = true;
		break;
	default:
		assert(0);
	}
}

}
}