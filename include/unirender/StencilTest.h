#pragma once

#include "unirender/StencilTestFace.h"

namespace ur
{

struct StencilTest
{
    bool            enabled = false;
    StencilTestFace front_face;
    StencilTestFace back_face;

    bool operator == (const StencilTest& st) const {
        if (enabled != st.enabled) {
            return false;
        }
        if (enabled) {
            return front_face == st.front_face
                && back_face == st.back_face;
        }
        return true;
    }

    bool operator != (const StencilTest& st) const {
        return !operator == (st);
    }

}; // StencilTest

}