#pragma once

namespace ur
{

struct ColorMask
{
    ColorMask(bool red, bool green, bool blue, bool alpha)
        : r(red), g(green), b(blue), a(alpha)
    {
    }

    bool operator == (const ColorMask& mask) const {
        return r == mask.r
            && g == mask.g
            && b == mask.b
            && a == mask.a;
    }

    bool operator != (const ColorMask& mask) const {
        return !operator == (mask);
    }

    bool r = true;
    bool g = true;
    bool b = true;
    bool a = true;
};

}