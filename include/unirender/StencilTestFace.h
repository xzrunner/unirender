#pragma once

namespace ur
{

enum class StencilOperation
{
    Zero,
    Invert,
    Keep,
    Replace,
    Increment,
    Decrement,
    IncrementWrap,
    DecrementWrap
};

enum class StencilTestFunction
{
    Never,
    Less,
    Equal,
    LessThanOrEqual,
    Greater,
    NotEqual,
    GreaterThanOrEqual,
    Always,
};

struct StencilTestFace
{
    StencilOperation    stencil_fail_op            = StencilOperation::Keep;
    StencilOperation    depth_fail_stencil_pass_op = StencilOperation::Keep;
    StencilOperation    depth_pass_stencil_pass_op = StencilOperation::Keep;
    StencilTestFunction function                   = StencilTestFunction::Always;
    int                 reference_value            = 0;
    int                 mask                       = ~0;

    bool operator == (const StencilTestFace& stf) const {
        return stencil_fail_op == stf.stencil_fail_op
            && depth_fail_stencil_pass_op == stf.depth_fail_stencil_pass_op
            && depth_pass_stencil_pass_op == stf.depth_pass_stencil_pass_op
            && function == stf.function
            && reference_value == stf.reference_value
            && mask == stf.mask;
    }

    bool operator != (const StencilTestFace& stf) const {
        return !operator == (stf);
    }

}; // StencilTestFace

}