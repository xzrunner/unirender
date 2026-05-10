#pragma once

#include "unirender/VertexArray.h"
#include "unirender/VertexInputAttribute.h"

#include <memory>
#include <vector>

namespace ur
{
namespace metal
{

class VertexArray : public ur::VertexArray
{
public:
    VertexArray() = default;
    virtual ~VertexArray() = default;

    virtual const std::shared_ptr<ur::VertexBuffer> GetVertexBuffer() const override { return m_vbuf; }
    virtual void SetVertexBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) override { m_vbuf = buf; }

    virtual std::vector<std::shared_ptr<ur::VertexInputAttribute>> GetVertexBufferAttrs() const override { return m_attrs; }
    virtual void SetVertexBufferAttrs(const std::vector<std::shared_ptr<ur::VertexInputAttribute>>& attrs) override { m_attrs = attrs; }

    virtual const std::shared_ptr<ur::IndexBuffer> GetIndexBuffer() const override { return m_ibuf; }
    virtual void SetIndexBuffer(const std::shared_ptr<ur::IndexBuffer>& buf) override { m_ibuf = buf; }

    virtual void SetInstanceBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) override { m_inst_buf = buf; }
    std::shared_ptr<ur::VertexBuffer> GetInstanceBuffer() const { return m_inst_buf; }

private:
    std::shared_ptr<ur::VertexBuffer> m_vbuf = nullptr;
    std::shared_ptr<ur::IndexBuffer>  m_ibuf = nullptr;
    std::shared_ptr<ur::VertexBuffer> m_inst_buf = nullptr;
    std::vector<std::shared_ptr<ur::VertexInputAttribute>> m_attrs;

}; // VertexArray

}
}
