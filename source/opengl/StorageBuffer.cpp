#include "unirender/opengl/StorageBuffer.h"

namespace ur
{
namespace opengl
{

StorageBuffer::StorageBuffer(BufferUsageHint usage_hint, int size_in_bytes)
	: m_buf(BufferTarget::ShaderStorageBuffer, usage_hint, size_in_bytes)
{
}

void StorageBuffer::ReadFromMemory(const void* data, int size, int offset)
{
	m_buf.ReadFromMemory(data, size, offset);
}

void* StorageBuffer::WriteToMemory(int size, int offset)
{
	return m_buf.WriteToMemory(size, offset);
}

void StorageBuffer::Bind() const
{
	m_buf.Bind();
}

void StorageBuffer::UnBind() const
{
	m_buf.UnBind();
}

void StorageBuffer::BindIndex(int idx)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, idx, m_buf.GetID());
}

}
}