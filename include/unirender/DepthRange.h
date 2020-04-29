#pragma once

namespace ur
{

struct DepthRange
{
    double d_near = 0.0;
    double d_far  = 1.0;

    bool operator == (const DepthRange& dr) const {
        return d_near == dr.d_near
            && d_far == dr.d_far;
    }

    bool operator != (const DepthRange& dr) const {
        return !operator == (dr);
    }

}; // DepthRange

}
