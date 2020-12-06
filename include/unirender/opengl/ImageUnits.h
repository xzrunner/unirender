#pragma once

#include <vector>
#include <memory>

namespace ur
{

class Device;

namespace opengl
{

class ImageUnit;

class ImageUnits
{
public:
	ImageUnits(const ur::Device& device);

    auto GetUnit(size_t slot) const {
        return slot < m_image_units.size() ? m_image_units[slot] : nullptr;
    }

    void Clean();

private:
    std::vector<std::shared_ptr<ImageUnit>> m_image_units;

}; // ImageUnits

}
}