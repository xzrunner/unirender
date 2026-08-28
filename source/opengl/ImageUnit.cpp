#include "unirender/opengl/ImageUnit.h"
#include "unirender/opengl/opengl.h"
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

void ImageUnit::SetAccess(ur::AccessType access) 
{
    if (m_access == access) {
        return;
    }
    m_access = access; 
    m_texture_dirty = true;
}

void ImageUnit::Clean()
{
    if (m_texture_dirty)
    {
        if (m_texture) {
            m_texture->BindToImage(m_index, m_access);
        } else {
            GLint major = 0;
            GLint minor = 0;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            if ((major > 4 || (major == 4 && minor >= 2)) && glBindImageTexture)
            {
                glBindImageTexture(m_index, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
            }
        }
        m_texture_dirty = false;
    }
}

void ImageUnit::Invalidate()
{
    m_texture_dirty = true;
}

}
}