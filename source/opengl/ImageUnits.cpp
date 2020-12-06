#include "unirender/opengl/ImageUnits.h"
#include "unirender/opengl/ImageUnit.h"
#include "unirender/Device.h"

namespace ur
{
namespace opengl
{

ImageUnits::ImageUnits(const ur::Device& device)
{
    const auto num = device.GetMaxNumImgUnits();
    m_image_units.resize(num);
    for (int i = 0; i < num; ++i) {
        m_image_units[i] = std::make_shared<ImageUnit>(i);
    }
}

void ImageUnits::Clean()
{
    if (m_image_units.empty()) {
        return;
    }

    for (auto& img : m_image_units) {
        img->Clean();
    }
}

}
}