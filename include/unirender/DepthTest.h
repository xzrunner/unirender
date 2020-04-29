#pragma once

namespace ur
{

enum class DepthTestFunc
{
    Never,
    Less,
    Equal,
    LessThanOrEqual,
    Greater,
    NotEqual,
    GreaterThanOrEqual,
    Always
};

struct DepthTest
{
    bool          enabled  = true;
    DepthTestFunc function = DepthTestFunc::Less;

    bool operator == (const DepthTest& dt) const {
        if (enabled != dt.enabled) {
            return false;
        }
        if (enabled) {
            return function == dt.function;
        }
        return true;
    }

    bool operator != (const DepthTest& dt) const {
        return !operator == (dt);
    }

}; // DepthTest

}