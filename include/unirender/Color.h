#pragma once

#include <cstdint>

namespace ur
{

struct Color
{
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        : r(r), g(g), b(b), a(a) {}

    bool operator == (const Color& col) const {
        return r == col.r
            && g == col.g
            && b == col.b
            && a == col.a;
    }

    bool operator != (const Color& col) const {
        return !operator == (col);
    }

    void FromRGBA(uint32_t rgba)
    {
	    r = (rgba >> 24) & 0xff;
	    g = (rgba >> 16) & 0xff;
	    b = (rgba >> 8)  & 0xff;
	    a = rgba         & 0xff;
    }

    uint8_t r = 0, g = 0, b = 0, a = 0;

}; // Color

}