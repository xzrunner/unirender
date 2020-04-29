#pragma once

namespace ur
{

struct Rectangle
{
    Rectangle() {}
    Rectangle(int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h) {}

    bool operator == (const Rectangle& rect) const {
        return x == rect.x
            && y == rect.y
            && w == rect.w
            && h == rect.h;
    }

    bool operator != (const Rectangle& rect) const {
        return !operator == (rect);
    }

    int x = 0, y = 0;
    int w = 0, h = 0;

}; // Rectangle

}