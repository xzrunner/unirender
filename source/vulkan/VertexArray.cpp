#include "unirender/vulkan/VertexArray.h"

namespace ur
{
namespace vulkan
{

VertexArray::VertexArray(const ur::Device& device)
{

}

VertexArray::~VertexArray()
{

}

void VertexArray::SetVertexBuffer(const std::shared_ptr<ur::VertexBuffer>& buf)
{
	m_vbuf = buf;
}

void VertexArray::SetVertexBufferAttrs(const std::vector<std::shared_ptr<ur::VertexBufferAttribute>>& attrs)
{

}

void VertexArray::SetIndexBuffer(const std::shared_ptr<ur::IndexBuffer>& buf)
{
	m_ibuf = buf;
}

}
}