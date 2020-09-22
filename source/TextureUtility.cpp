#include "unirender/TextureUtility.h"

namespace ur
{

int TextureUtility::RequiredSizeInBytes(int width, int height, TextureFormat fmt, int row_alignment)
{
    float pixel_size = 0;
    switch (fmt)
    {
    case TextureFormat::RGBA8:
        pixel_size = 4;
        break;
    case TextureFormat::RGBA4:
        pixel_size = 2;
        break;
    case TextureFormat::RGB:
        pixel_size = 3;
        break;
    case TextureFormat::RGB565:
        pixel_size = 2;
        break;
    case TextureFormat::BGRA_EXT:
        pixel_size = 4;
        break;
    case TextureFormat::BGR_EXT:
        pixel_size = 3;
        break;
    case TextureFormat::RGBA16F:
        pixel_size = 8;
        break;
    case TextureFormat::RGBA32F:
        pixel_size = 16;
        break;
    case TextureFormat::RGB16F:
        pixel_size = 6;
        break;
    case TextureFormat::RGB32F:
        pixel_size = 12;
        break;
    case TextureFormat::RG16F:
        pixel_size = 4;
        break;
    case TextureFormat::A8:
        pixel_size = 1;
        break;
    case TextureFormat::RED:
        pixel_size = 1;
        break;
    case TextureFormat::R16:
        pixel_size = 2;
        break;
    case TextureFormat::DEPTH:
        pixel_size = 1;
        break;
    case TextureFormat::RGB32I:
        pixel_size = 12;
        break;
    case TextureFormat::PVR2:
        pixel_size = 1.0f / 4;
        break;
    case TextureFormat::PVR4:
        pixel_size = 1.0f / 2;
        break;
    case TextureFormat::ETC1:
        pixel_size = 1.0f / 2;
        break;
    case TextureFormat::ETC2:
        pixel_size = 1;
        break;
    case TextureFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT:
        pixel_size = 1.0f / 2;
        break;
    case TextureFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT:
        pixel_size = 1;
        break;
    case TextureFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT:
        pixel_size = 1;
        break;
    }

    int row_size = static_cast<int>(width * pixel_size);

    int remainder = (row_size % row_alignment);
    row_size += (row_alignment - remainder) % row_alignment;

    return row_size * height;
}

}