#pragma once

#include "unirender/ScissorTest.h"
#include "unirender/ColorMask.h"
#include "unirender/Color.h"

namespace ur
{

enum class ClearBuffers
{
    ColorBuffer         = 1,
    DepthBuffer         = 2,
    StencilBuffer       = 4,
    ColorAndDepthBuffer = ColorBuffer | DepthBuffer,
    All                 = ColorBuffer | DepthBuffer | StencilBuffer
};

struct ClearState
{
    ScissorTest scissor_test;
    ColorMask   color_mask         = ColorMask(true, true, true, true);
    bool        depth_mask         = true;
    int         front_stencil_mask = ~0;
    int         back_stencil_mask  = ~0;

    ClearBuffers buffers = ClearBuffers::All;
    Color        color   = Color(255, 255, 255, 255);
    double       depth   = 1;
    int          stencil = 0;

}; // ClearState

}