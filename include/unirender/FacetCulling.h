#pragma once

#include "unirender/WindingOrder.h"

namespace ur
{

enum class CullFace
{
    Front,
    Back,
    FrontAndBack
};

struct FacetCulling
{
    bool     enabled = true;
    CullFace face    = CullFace::Back;

    WindingOrder front_face_winding_order = WindingOrder::Counterclockwise;

    bool operator == (const FacetCulling& fc) const {
        if (enabled != fc.enabled) {
            return false;
        }
        if (enabled) {
            return face == fc.face
                && front_face_winding_order == fc.front_face_winding_order;
        }
        return true;
    }

    bool operator != (const FacetCulling& fc) const {
        return !operator == (fc);
    }

}; // FacetCulling

}