#include "unirender/opengl/ImageUnit.h"
#include "unirender/Texture.h"

namespace ur
{
namespace opengl
{

ImageUnit::ImageUnit(int index)
    : m_index(index)
{
}

void ImageUnit::SetTexture(const std::shared_ptr<Texture>& texture)
{
    if (m_texture == texture) {
        return;
    }
    m_texture = texture;
    m_texture_dirty = true;
}

void ImageUnit::Clean()
{
    if (m_texture_dirty)
    {
        if (m_texture) {
            m_texture->BindToImage(m_index, m_access);
        }
        m_texture_dirty = false;
    }
}

}
}