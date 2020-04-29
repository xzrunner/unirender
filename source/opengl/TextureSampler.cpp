#include "unirender/opengl/TextureSampler.h"
#include "unirender/opengl/TypeConverter.h"

#include <assert.h>

namespace ur
{
namespace opengl
{

TextureSampler::TextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter,
                               TextureWrap wrap_s, TextureWrap wrap_t, float max_anistropy)
    : ur::TextureSampler(min_filter, mag_filter, wrap_s, wrap_t, max_anistropy)
{
    glGenSamplers(1, &m_id);

    glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, TypeConverter::To(min_filter));
    glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, TypeConverter::To(mag_filter));
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_S,     TypeConverter::To(wrap_s));
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_T,     TypeConverter::To(wrap_t));
}

TextureSampler::~TextureSampler()
{
    glDeleteSamplers(1, &m_id);
}

void TextureSampler::Bind(int tex_unit_idx)
{
    glBindSampler(tex_unit_idx, m_id);
}

void TextureSampler::UnBind(int tex_unit_idx)
{
    glBindSampler(tex_unit_idx, 0);
}

}
}