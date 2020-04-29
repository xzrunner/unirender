#pragma once

#include "unirender/Rectangle.h"

namespace ur
{

struct ScissorTest
{
    bool enabled = false;

    Rectangle rect;

    bool operator == (const ScissorTest& st) const {
        if (enabled != st.enabled) {
            return false;
        }
        if (enabled) {
            return rect == st.rect;
        }
        return true;
    }

    bool operator != (const ScissorTest& st) const {
        return !operator == (st);
    }

}; // ScissorTest

}