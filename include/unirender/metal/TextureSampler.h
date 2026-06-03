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

    // GetMinFilter/GetMagFilter/GetWrapS/GetWrapT are non-virtual on the base, which
    // stores these now; don't re-declare or re-store them here.
    virtual void Bind(int tex_unit_idx) override {}

    void* GetMTLSamplerState() const { return m_mtl_sampler; }

private:
    void* m_mtl_sampler = nullptr; // id<MTLSamplerState>

}; // TextureSampler

}
}
