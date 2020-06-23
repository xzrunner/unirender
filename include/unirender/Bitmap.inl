#pragma once

namespace ur
{

template<typename T>
Bitmap<T>::Bitmap(size_t width, size_t height,
	              size_t channels, const T* pixels)
    : m_width(width)
    , m_height(height)
    , m_channels(channels)
{
    auto sz = m_width * m_height * channels;
    try {
        m_pixels = new T[sz];
        memcpy(m_pixels, pixels, sz);
    } catch (...) {
        m_pixels = nullptr;
    }
}

template<typename T>
Bitmap<T>::~Bitmap()
{
    delete[] m_pixels;
}

}