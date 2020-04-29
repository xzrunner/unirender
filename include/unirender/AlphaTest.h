#pragma once

namespace ur
{

enum class AlphaTestFunc
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

struct AlphaTest
{
    bool          enabled  = true;
    AlphaTestFunc function = AlphaTestFunc::Always;
    float         ref      = 0;

    bool operator == (const AlphaTest& at) const {
        if (enabled != at.enabled) {
            return false;
        }
        if (enabled) {
            return function == at.function
                && ref == at.ref;
        }
        return true;
    }

    bool operator != (const AlphaTest& at) const {
        return !operator == (at);
    }

}; // AlphaTest

}