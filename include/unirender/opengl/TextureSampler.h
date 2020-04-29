#pragma once

#include "unirender/TextureSampler.h"
#include "unirender/opengl/opengl.h"

namespace ur
{
namespace opengl
{

class TextureSampler : public ur::TextureSampler
{
public:
    TextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter,
        TextureWrap wrap_s, TextureWrap wrap_t, float max_anistropy);
    virtual ~TextureSampler();

    virtual void Bind(int tex_unit_idx) override;
    static void UnBind(int tex_unit_idx);

private:
    GLuint m_id = 0;

}; // TextureSampler

}
}