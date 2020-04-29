#include "unirender/Bitmap.h"

#include <cstring>

namespace ur
{

Bitmap::Bitmap(size_t width, size_t height, size_t channels,
               const unsigned char* pixels)
    : m_width(width)
    , m_height(height)
    , m_channels(channels)
{
    auto sz = m_width * m_height * channels;
    try {
        m_pixels = new unsigned char[sz];
        memcpy(m_pixels, pixels, sz);
    } catch (...) {
        m_pixels = nullptr;
    }
}

Bitmap::~Bitmap()
{
    delete[] m_pixels;
}

}