#pragma once

#include "unirender/Color.h"

namespace ur
{

enum class BlendingFactor
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
    Src1Color,
    OneMinusSrc1Color,
    Src1Alpha,
    OneMinusSrc1Alpha,
};

enum class BlendEquation
{
    Add,
    Minimum,
    Maximum,
    Subtract,
    ReverseSubtract
};

struct Blending
{
    bool enabled = false;

    bool separately = false;

    // separately
    BlendingFactor src_rgb        = BlendingFactor::One;
    BlendingFactor src_alpha      = BlendingFactor::One;
    BlendingFactor dst_rgb        = BlendingFactor::Zero;
    BlendingFactor dst_alpha      = BlendingFactor::Zero;
    BlendEquation  rgb_equation   = BlendEquation::Add;
    BlendEquation  alpha_equation = BlendEquation::Add;
    Color          color          = Color(0, 0, 0, 0);

    // no separately
    BlendingFactor src      = BlendingFactor::One;
    BlendingFactor dst      = BlendingFactor::Zero;
    BlendEquation  equation = BlendEquation::Add;

    bool operator == (const Blending& blend) const
    {
        if (enabled != blend.enabled ||
            separately != blend.separately) {
            return false;
        }

        if (enabled)
        {
            if (separately)
            {
                return src_rgb == blend.src_rgb
                    && src_alpha == blend.src_alpha
                    && dst_rgb == blend.dst_rgb
                    && dst_alpha == blend.dst_alpha
                    && rgb_equation == blend.rgb_equation
                    && alpha_equation == blend.alpha_equation
                    && color == blend.color;
            }
            else
            {
                return src == blend.src
                    && dst == blend.dst
                    && equation == blend.equation;
            }
        }
        return true;
    }

    bool operator != (const Blending& blend) const {
        return !operator == (blend);
    }

}; // Blending

}