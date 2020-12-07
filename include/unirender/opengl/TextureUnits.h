#pragma once

#include "unirender/typedef.h"

#include <vector>
#include <memory>

namespace ur
{

class Device;

namespace opengl
{

class TextureUnit;
class TextureUnits
{
public:
    TextureUnits(const ur::Device& device);

    auto GetUnit(size_t slot) const {
        return slot < m_texture_units.size() ? m_texture_units[slot] : nullptr;
    }

    void Clean();

private:
    std::vector<std::shared_ptr<TextureUnit>> m_texture_units;

}; // TextureUnits

}
}