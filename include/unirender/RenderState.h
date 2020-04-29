#pragma once

#include "unirender/Blending.h"
#include "unirender/ColorMask.h"
#include "unirender/DepthRange.h"
#include "unirender/DepthTest.h"
#include "unirender/FacetCulling.h"
#include "unirender/PrimitiveRestart.h"
#include "unirender/ScissorTest.h"
#include "unirender/StencilTest.h"
#include "unirender/AlphaTest.h"

namespace ur
{

enum class ProgramPointSize
{
    Enabled,
    Disabled
};

enum class RasterizationMode
{
    Point,
    Line,
    Fill
};

struct RenderState
{
    PrimitiveRestart  prim_restart;
    FacetCulling      facet_culling;
    ProgramPointSize  prog_point_size    = ProgramPointSize::Disabled;
    RasterizationMode rasterization_mode = RasterizationMode::Fill;
    ScissorTest       scissor_test;
    StencilTest       stencil_test;
    DepthTest         depth_test;
    DepthRange        depth_range;
    Blending          blending;
    ColorMask         color_mask = ColorMask(true, true, true, true);
    bool              depth_mask = true;
    AlphaTest         alpha_test;

    bool operator == (const RenderState& rs) const {
        return prim_restart == rs.prim_restart
            && facet_culling == rs.facet_culling
            && prog_point_size == rs.prog_point_size
            && rasterization_mode == rs.rasterization_mode
            && scissor_test == rs.scissor_test
            && stencil_test == rs.stencil_test
            && depth_test == rs.depth_test
            && depth_range == rs.depth_range
            && blending == rs.blending
            && color_mask == rs.color_mask
            && depth_mask == rs.depth_mask
            && alpha_test == rs.alpha_test;
    }

    bool operator != (const RenderState& rs) const {
        return !operator == (rs);
    }

}; // RenderState

}