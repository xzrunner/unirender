#include "unirender/Device.h"

#include <assert.h>

namespace ur
{

Device::Device(std::ostream& logger) 
    : m_logger(logger) 
{
}

void Device::Init()
{
    m_nearest_clamp = CreateTextureSampler(
        TextureMinificationFilter::Nearest,
        TextureMagnificationFilter::Nearest,
        TextureWrap::ClampToEdge,
        TextureWrap::ClampToEdge,
        1.0f
    );
    m_linear_clamp = CreateTextureSampler(
        TextureMinificationFilter::Linear,
        TextureMagnificationFilter::Linear,
        TextureWrap::ClampToEdge,
        TextureWrap::ClampToEdge,
        1.0f
    );
    m_nearest_repeat = CreateTextureSampler(
        TextureMinificationFilter::Nearest,
        TextureMagnificationFilter::Nearest,
        TextureWrap::Repeat,
        TextureWrap::Repeat,
        1.0f
    );
    m_linear_repeat = CreateTextureSampler(
        TextureMinificationFilter::Linear,
        TextureMagnificationFilter::Linear,
        TextureWrap::Repeat,
        TextureWrap::Repeat,
        1.0f
    );
}

std::shared_ptr<TextureSampler> 
Device::GetTextureSampler(TextureSamplerType type) const
{
    switch (type)
    {
    case TextureSamplerType::NearestClamp:
        return m_nearest_clamp;
    case TextureSamplerType::LinearClamp:
        return m_linear_clamp;
    case TextureSamplerType::NearestRepeat:
        return m_nearest_repeat;
    case TextureSamplerType::LinearRepeat:
        return m_linear_repeat;
    default:
        assert(0);
        return nullptr;
    }
}

}