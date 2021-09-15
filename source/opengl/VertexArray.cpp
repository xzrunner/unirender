#include "unirender/opengl/VertexArray.h"
#include "unirender/opengl/IndexBuffer.h"
#include "unirender/VertexBuffer.h"

namespace ur
{
namespace opengl
{

VertexArray::VertexArray(const ur::Device& device)
    : m_vbuf_attrs(device)
{
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_id);
}

void VertexArray::SetVertexBuffer(const std::shared_ptr<ur::VertexBuffer>& buf)
{
    if (m_vbuf != buf)
    {
        m_vbuf = buf;
        m_vbuf_attrs.SetVertexBuffer(buf);

        m_vbuf_dirty = true;
    }
}

std::vector<std::shared_ptr<ur::VertexInputAttribute>> VertexArray::GetVertexBufferAttrs() const
{
    std::vector<std::shared_ptr<ur::VertexInputAttribute>> ret;
    auto& attrs = m_vbuf_attrs.GetAttrs();
    for (auto& a : attrs) {
        ret.push_back(a.attr);
    }

    while (!ret.back()) {
        ret.pop_back();
    }

    return ret;
}

void VertexArray::SetVertexBufferAttrs(const std::vector<std::shared_ptr<ur::VertexInputAttribute>>& attrs)
{
    m_vbuf_attrs.SetAttrs(attrs);
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<ur::IndexBuffer>& buf)
{
    if (m_ibuf != buf) {
        m_ibuf = buf;
        m_ibuf_dirty = true;
    }
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_id);
}

void VertexArray::Clean()
{
    m_vbuf_attrs.Clean();

    if (m_ibuf_dirty)
    {
        if (m_ibuf) {
            m_ibuf->Bind();
        } else {
            IndexBuffer::UnBind();
        }
        m_ibuf_dirty = false;
    }
}

}
}