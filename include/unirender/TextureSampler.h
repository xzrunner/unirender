#pragma once

#include "unirender/TextureMinificationFilter.h"
#include "unirender/TextureMagnificationFilter.h"
#include "unirender/TextureWrap.h"

namespace ur
{

class TextureSampler
{
public:
    TextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter,
        TextureWrap wrap_s, TextureWrap wrap_t, float max_anistropy = 1)
        : m_min_filter(min_filter)
        , m_mag_filter(mag_filter)
        , m_wrap_s(wrap_s)
        , m_wrap_t(wrap_t)
        , m_max_anistropy(max_anistropy)
    {
    }
    virtual ~TextureSampler() {}

    virtual void Bind(int tex_unit_idx) = 0;

    auto GetMinFilter() const { return m_min_filter; }
    auto GetMagFilter() const { return m_mag_filter; }
    auto GetWrapS() const { return m_wrap_s; }
    auto GetWrapT() const { return m_wrap_t; }

private:
    TextureMinificationFilter  m_min_filter = TextureMinificationFilter::Nearest;
    TextureMagnificationFilter m_mag_filter = TextureMagnificationFilter::Nearest;
    TextureWrap                m_wrap_s     = TextureWrap::Repeat;
    TextureWrap                m_wrap_t     = TextureWrap::Repeat;
    float                      m_max_anistropy = 1;

}; // TextureSampler

}