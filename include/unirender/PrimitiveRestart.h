#pragma once

namespace ur
{

struct PrimitiveRestart
{
    bool enabled = false;
    int  index   = 0;

    bool operator == (const PrimitiveRestart& pr) const {
        if (enabled != pr.enabled) {
            return false;
        }
        if (enabled) {
            return index == pr.index;
        }
        return true;
    }

    bool operator != (const PrimitiveRestart& pr) const {
        return !operator == (pr);
    }

}; // PrimitiveRestart

}