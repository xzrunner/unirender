#pragma once

namespace ur
{

class Bitmap
{
public:
    Bitmap(size_t width, size_t height,
        size_t channels, const unsigned char* pixels);
    ~Bitmap();

    int CalcSizeInBytes() const {
        return m_width * m_height * m_channels;
    }

    auto GetWidth() const { return m_width; }
    auto GetHeight() const { return m_height; }

    auto GetPixels() const { return m_pixels; }

private:
    size_t m_width = 0, m_height = 0;
    size_t m_channels = 3;

    unsigned char* m_pixels = nullptr;

}; // Bitmap

}