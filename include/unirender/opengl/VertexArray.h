#pragma once

#include "unirender/VertexArray.h"
#include "unirender/opengl/opengl.h"
#include "unirender/opengl/VertexInputAttributes.h"

namespace ur
{

namespace opengl
{

class VertexArray : public ur::VertexArray
{
public:
    VertexArray(const ur::Device& device);
    virtual ~VertexArray();

    virtual const std::shared_ptr<ur::VertexBuffer> GetVertexBuffer() const override { return m_vbuf; }
    virtual void SetVertexBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) override;
    virtual std::vector<std::shared_ptr<ur::VertexInputAttribute>> GetVertexBufferAttrs() const override;
    virtual void SetVertexBufferAttrs(const std::vector<std::shared_ptr<ur::VertexInputAttribute>>& attrs) override;

    virtual const std::shared_ptr<ur::IndexBuffer> GetIndexBuffer() const override { return m_ibuf; }
    virtual void SetIndexBuffer(const std::shared_ptr<ur::IndexBuffer>& buf) override;

    virtual void SetInstanceBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) override { m_vbuf_attrs.SetInstanceBuffer(buf); }

    int GetMaxArrayIndex() const { return m_vbuf_attrs.GetMaxArrayIndex(); }

    void Bind() const;

    void Clean();

private:
    GLuint m_id = 0;

    std::shared_ptr<ur::VertexBuffer> m_vbuf = nullptr;
    VertexInputAttributes m_vbuf_attrs;
    bool m_vbuf_dirty = true;

    std::shared_ptr<ur::IndexBuffer> m_ibuf = nullptr;
    bool m_ibuf_dirty = true;

}; // VertexArray

}
}