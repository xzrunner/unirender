#pragma once

#include "unirender/RenderState.h"

#include <memory>

namespace ur
{

class ShaderProgram;
class VertexArray;

struct DrawState
{
    RenderState render_state;
    std::shared_ptr<ShaderProgram> program      = nullptr;
    std::shared_ptr<VertexArray>   vertex_array = nullptr;

    int offset = 0;
    int count = 0;

}; // DrawState

}