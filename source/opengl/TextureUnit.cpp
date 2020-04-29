#include "unirender/opengl/TextureUnit.h"
#include "unirender/opengl/Texture.h"
#include "unirender/opengl/TextureSampler.h"

namespace ur
{
namespace opengl
{

TextureUnit::TextureUnit(int index)
    : m_index(index)
    , m_slot(GL_TEXTURE0 + index)
{
}

void TextureUnit::SetTexture(const std::shared_ptr<ur::Texture>& texture)
{
    if (m_texture == texture) {
        return;
    }
    m_texture = texture;
    m_texture_dirty = true;
}

void TextureUnit::SetSampler(const std::shared_ptr<ur::TextureSampler>& sampler)
{
    if (m_sampler == sampler) {
        return;
    }
    m_sampler = sampler;
    m_sampler_dirty = true;
}

void TextureUnit::Clean()
{
    if (!m_texture_dirty && !m_sampler_dirty) {
        return;
    }

    glActiveTexture(m_slot);

    if (m_texture_dirty)
    {
        if (m_texture) {
            m_texture->Bind();
        } else {
            Texture::UnBind(TextureTarget::Texture2D);
        }
        m_texture_dirty = false;
    }

    if (m_sampler_dirty)
    {
        if (m_sampler) {
            m_sampler->Bind(m_index);
        } else {
            TextureSampler::UnBind(m_index);
        }
        m_sampler_dirty = false;
    }
}

void TextureUnit::CleanLastTextureUnit()
{
    if (m_texture) {
        m_texture_dirty = true;
    }
    Clean();
}

}
}