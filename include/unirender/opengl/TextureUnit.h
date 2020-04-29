#pragma once

#include "unirender/TextureUnit.h"
#include "unirender/opengl/opengl.h"

#include <memory>

namespace ur
{
namespace opengl
{

class TextureUnit : public ur::TextureUnit
{
public:
    TextureUnit(int index);

    virtual void SetTexture(const std::shared_ptr<ur::Texture>& texture) override;
    virtual void SetSampler(const std::shared_ptr<ur::TextureSampler>& sampler) override;

    void Clean();
    void CleanLastTextureUnit();

private:
    int m_index = 0;
    GLenum m_slot = 0;

    std::shared_ptr<Texture>        m_texture = nullptr;
    std::shared_ptr<TextureSampler> m_sampler = nullptr;

    bool m_texture_dirty = false;
    bool m_sampler_dirty = false;

}; // TextureUnit

}
}