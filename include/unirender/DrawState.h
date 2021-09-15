#pragma once

#ifdef DrawState
#undef DrawState
#endif

#include "unirender/RenderState.h"

#include <memory>

namespace ur
{

class ShaderProgram;
class VertexArray;
class DescriptorSet;
class PipelineLayout;
class Pipeline;

struct DrawState
{
    RenderState render_state;
    std::shared_ptr<ShaderProgram> program      = nullptr;
    std::shared_ptr<VertexArray>   vertex_array = nullptr;

    int offset = 0;
    int count = 0;

    int num_instances = 0;

    std::shared_ptr<DescriptorSet>  desc_set        = nullptr;
    std::shared_ptr<PipelineLayout> pipeline_layout = nullptr;
    std::shared_ptr<Pipeline>       pipeline        = nullptr;

}; // DrawState

}