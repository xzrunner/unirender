#pragma once

#include "unirender/TextureBuffer.h"
#include "unirender/TextureFormat.h"
#include "unirender/opengl/Buffer.h"

namespace ur
{

class Texture;
class Device;

namespace opengl
{

class TextureBuffer : public ur::TextureBuffer
{
public:
	TextureBuffer(BufferUsageHint usage_hint, int size_in_bytes,
        ur::TextureFormat format, const ur::Device& device);

    virtual void ReadFromMemory(const void* data, int size, int offset) override;

    virtual void Bind() const override;
    virtual void UnBind() const override;

    virtual std::shared_ptr<ur::Texture> GetTexture() const override { return m_tex; }

private:
	Buffer m_buf;

    std::shared_ptr<Texture> m_tex = nullptr;

}; // TextureBuffer

}
}