#include "unirender/opengl/TextureUnits.h"
#include "unirender/opengl/Device.h"
#include "unirender/opengl/TextureUnit.h"

namespace ur
{
namespace opengl
{

TextureUnits::TextureUnits(const ur::Device& device)
{
    const auto num = device.GetMaxNumTexUnits();
    m_texture_units.resize(num);
    for (int i = 0; i < num; ++i) {
        m_texture_units[i] = std::make_shared<TextureUnit>(i);
    }
}

void TextureUnits::Clean()
{
    if (m_texture_units.empty()) {
        return;
    }

    for (auto& tex : m_texture_units) {
        tex->Clean();
    }

    // CleanLastTextureUnit
    auto& last = m_texture_units.back();
    last->CleanLastTextureUnit();
}

}
}