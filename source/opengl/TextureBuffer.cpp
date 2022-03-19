#include "unirender/opengl/TextureBuffer.h"
#include "unirender/opengl/TextureFormat.h"
#include "unirender/opengl/Texture.h"
#include "unirender/Texture.h"
#include "unirender/TextureDescription.h"

namespace ur
{
namespace opengl
{

TextureBuffer::TextureBuffer(BufferUsageHint usage_hint, int size_in_bytes,
	                         ur::TextureFormat format, const ur::Device& device)
	: m_buf(BufferTarget::TextureBuffer, usage_hint, size_in_bytes)
{
	ur::TextureDescription desc;
	desc.target = ur::TextureTarget::TextureBuffer;
	m_tex = std::make_shared<ur::opengl::Texture>(desc, device);

	m_tex->Bind();

	TextureFormat fmt(format);
	glTexBuffer(GL_TEXTURE_BUFFER, fmt.internal_format, m_buf.GetID());
}

void TextureBuffer::ReadFromMemory(const void* data, int size, int offset)
{
	m_buf.ReadFromMemory(data, size, offset);
}

void TextureBuffer::Bind() const
{
	m_buf.Bind();
}

void TextureBuffer::UnBind() const
{
	m_buf.UnBind();
}

}
}