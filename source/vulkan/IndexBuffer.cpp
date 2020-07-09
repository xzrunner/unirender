#include "unirender/vulkan/IndexBuffer.h"

namespace ur
{
namespace vulkan
{

IndexBuffer::IndexBuffer(BufferUsageHint usage_hint, int size_in_bytes)
{
}

int IndexBuffer::GetSizeInBytes() const
{
	return 0;
}

BufferUsageHint IndexBuffer::GetUsageHint() const
{
    return BufferUsageHint::StreamDraw;
}

IndexBufferDataType IndexBuffer::GetDataType() const
{
    return IndexBufferDataType::UnsignedShort;
}

void IndexBuffer::ReadFromMemory(const void* data, int size, int offset)
{
}

void* IndexBuffer::WriteToMemory(int size, int offset)
{
	return nullptr;
}

void IndexBuffer::Bind() const
{
}

void IndexBuffer::UnBind()
{
}

void IndexBuffer::Reset(int size_in_bytes)
{
}

}
}