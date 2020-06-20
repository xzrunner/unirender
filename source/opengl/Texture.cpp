#include "unirender/opengl/Texture.h"
#include "unirender/opengl/TypeConverter.h"
#include "unirender/opengl/Device.h"
#include "unirender/opengl/WritePixelBuffer.h"
#include "unirender/opengl/ReadPixelBuffer.h"
#include "unirender/opengl/TextureFormat.h"
#include "unirender/TextureSampler.h"
#include "unirender/TextureUtility.h"

#include <assert.h>

namespace ur
{
namespace opengl
{

Texture::Texture(TextureDescription desc, const ur::Device& device)
    : m_desc(desc)
{
    glGenTextures(1, &m_id);

    m_last_tex_unit = GL_TEXTURE0 + (device.GetMaxNumTexUnits() - 1);

    WritePixelBuffer::UnBind();
    BindToLastTextureUnit();

    if (desc.width != 0 && desc.height != 0)
    {
        TextureFormat fmt(desc.format);
        glTexImage2D(TypeConverter::To(desc.target), 0, fmt.internal_format,
            desc.width, desc.height, 0, fmt.pixel_format, fmt.pixel_type, nullptr);
    }

    ApplySampler(device.GetTextureSampler(Device::TextureSamplerType::LinearClamp));
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

void Texture::Bind() const
{
    glBindTexture(TypeConverter::To(m_desc.target), m_id);
}

void Texture::UnBind(TextureTarget target)
{
    glBindTexture(TypeConverter::To(target), 0);
}

void Texture::Upload(const void* pixels, int x, int y, int w, int h, int miplevel, int row_alignment)
{
    BindToLastTextureUnit();

    glPixelStorei(GL_UNPACK_ALIGNMENT, row_alignment);

    TextureFormat fmt(m_desc.format);
    glTexSubImage2D(TypeConverter::To(m_desc.target), miplevel, x, y, w, h,
        fmt.pixel_format, fmt.pixel_type, pixels);
}

void Texture::ApplySampler(const std::shared_ptr<ur::TextureSampler>& sampler)
{
    auto target = TypeConverter::To(m_desc.target);

    auto min_filter = TypeConverter::To(sampler->GetMinFilter());
    auto mag_filter = TypeConverter::To(sampler->GetMagFilter());
    auto wrap_s = TypeConverter::To(sampler->GetWrapS());
    auto wrap_t = TypeConverter::To(sampler->GetWrapT());

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);
}

bool Texture::ReadFromMemory(const ur::WritePixelBuffer& buf, int x, int y,
                             int w, int h, int row_alignment)
{
    if (x < 0 || y < 0 ||
        x + w > m_desc.width ||
        y + h > m_desc.height) {
        return false;
    }

    TextureFormat fmt(m_desc.format);
    const auto size = TextureUtility::RequiredSizeInBytes(w, h, m_desc.format, row_alignment);
    if (buf.GetSizeInBytes() < size) {
        return false;
    }

    VerifyRowAlignment(row_alignment);

    auto& buf_gl = static_cast<const WritePixelBuffer&>(buf);
    buf_gl.Bind();
    BindToLastTextureUnit();

    glPixelStorei(GL_UNPACK_ALIGNMENT, row_alignment);

    glTexSubImage2D(TypeConverter::To(m_desc.target), 0, x, y, w, h,
        fmt.pixel_format, fmt.pixel_type, nullptr);

    if (m_desc.gen_mipmaps) {
        assert(m_desc.target == TextureTarget::Texture2D);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return true;
}

void Texture::ReadFromMemory(const void* pixels, ur::TextureFormat tex_fmt,
                             int width, int height, int depth, int row_alignment, int mip_level)
{
    m_desc.format = tex_fmt;

    m_desc.width  = width;
    m_desc.height = height;
    m_desc.depth  = depth;

    VerifyRowAlignment(row_alignment);

    BindToLastTextureUnit();

    glPixelStorei(GL_UNPACK_ALIGNMENT, row_alignment);

    TextureFormat fmt(m_desc.format);
    auto target = TypeConverter::To(m_desc.target);
    switch (m_desc.target)
    {
    case TextureTarget::Texture2D:
        glTexImage2D(target, mip_level, fmt.internal_format,
            width, height, 0, fmt.pixel_format, fmt.pixel_type, pixels);
        break;
    case TextureTarget::Texture3D:
        glTexImage3D(target, mip_level, fmt.internal_format,
            width, height, depth, 0, fmt.pixel_format, fmt.pixel_type, pixels);
        break;
    case TextureTarget::TextureCubeMap:
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, fmt.internal_format,
                width, height, 0, fmt.pixel_format, fmt.pixel_type, pixels);
        }
        break;
    default:
        assert(0);
    }

    if (m_desc.gen_mipmaps) {
        glGenerateMipmap(target);
    }
}

std::shared_ptr<ur::ReadPixelBuffer>
Texture::WriteToMemory(int row_alignment)
{
    VerifyRowAlignment(row_alignment);

    const auto size = TextureUtility::RequiredSizeInBytes(m_desc.width, m_desc.height, m_desc.format, row_alignment);
    auto pixel_buf = std::make_shared<ReadPixelBuffer>(BufferUsageHint::StreamRead, size);

    pixel_buf->Bind();
    BindToLastTextureUnit();

    glPixelStorei(GL_PACK_ALIGNMENT, row_alignment);

    TextureFormat fmt(m_desc.format);
    glGetTexImage(TypeConverter::To(m_desc.target), 0, fmt.pixel_format, fmt.pixel_type, nullptr);

    return pixel_buf;
}

void Texture::BindToLastTextureUnit()
{
    glActiveTexture(m_last_tex_unit);
    Bind();
}

void Texture::VerifyRowAlignment(int row_alignment)
{
    if ((row_alignment != 1) &&
        (row_alignment != 2) &&
        (row_alignment != 4) &&
        (row_alignment != 8))
    {
        throw std::runtime_error("row_alignment");
    }
}

}
}