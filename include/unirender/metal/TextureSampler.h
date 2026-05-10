#pragma once

#include "unirender/TextureSampler.h"

namespace ur
{
namespace metal
{

class TextureSampler : public ur::TextureSampler
{
public:
    TextureSampler(void* mtl_device,
                   TextureMinificationFilter min_filter,
                   TextureMagnificationFilter mag_filter,
                   TextureWrap wrap_s, TextureWrap wrap_t,
                   float max_anisotropy);
    virtual ~TextureSampler();

    virtual TextureMinificationFilter  GetMinFilter() const override { return m_min_filter; }
    virtual TextureMagnificationFilter GetMagFilter() const override { return m_mag_filter; }
    virtual TextureWrap GetWrapS() const override { return m_wrap_s; }
    virtual TextureWrap GetWrapT() const override { return m_wrap_t; }

    virtual void Bind(int tex_unit_idx) override {}

    void* GetMTLSamplerState() const { return m_mtl_sampler; }

private:
    void* m_mtl_sampler = nullptr; // id<MTLSamplerState>

    TextureMinificationFilter  m_min_filter;
    TextureMagnificationFilter m_mag_filter;
    TextureWrap m_wrap_s;
    TextureWrap m_wrap_t;

}; // TextureSampler

}
}
