#pragma once

#include "unirender/VertexArray.h"

namespace ur
{

class Device;

namespace vulkan
{

class VertexArray : public ur::VertexArray
{
public:
	VertexArray(const ur::Device& device);
	virtual ~VertexArray();

	virtual const std::shared_ptr<ur::VertexBuffer> GetVertexBuffer() const override { return m_vbuf; }
	virtual void SetVertexBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) override;
	virtual void SetVertexBufferAttrs(const std::vector<std::shared_ptr<ur::VertexBufferAttribute>>& attrs) override;

	virtual const std::shared_ptr<ur::IndexBuffer> GetIndexBuffer() const override { return m_ibuf; }
	virtual void SetIndexBuffer(const std::shared_ptr<ur::IndexBuffer>& buf) override;

private:
	std::shared_ptr<ur::VertexBuffer> m_vbuf = nullptr;
	std::shared_ptr<ur::IndexBuffer> m_ibuf = nullptr;

}; // VertexArray

}
}