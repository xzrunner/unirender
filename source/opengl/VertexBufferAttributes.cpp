#include "unirender/opengl/VertexBufferAttributes.h"
#include "unirender/opengl/opengl.h"
#include "unirender/opengl/VertexBuffer.h"
#include "unirender/opengl/TypeConverter.h"
#include "unirender/VertexBufferAttribute.h"
#include "unirender/Device.h"

#include <algorithm>

#include <assert.h>

namespace ur
{
namespace opengl
{

VertexBufferAttributes::VertexBufferAttributes(const Device& device)
{
    m_attrs.resize(device.GetMaxNumVertAttrs());
}

void VertexBufferAttributes::SetAttrs(const std::vector<std::shared_ptr<ur::VertexBufferAttribute>>& attrs)
{
    assert(attrs.size() <= m_attrs.size());
    for (size_t i = 0, n = attrs.size(); i < n; ++i) {
        if (m_attrs[i].attr != attrs[i]) {
            m_attrs[i].attr = attrs[i];
            m_attrs[i].dirty = true;
        }
    }
    for (size_t i = attrs.size(), n = m_attrs.size(); i < n; ++i) {
        if (m_attrs[i].attr) {
            m_attrs[i].attr = nullptr;
            m_attrs[i].dirty = true;
        }
    }
}

void VertexBufferAttributes::Clean()
{
    for (size_t i = 0, n = m_attrs.size(); i < n; ++i)
    {
        auto& attr = m_attrs[i];
        if (attr.dirty)
        {
            if (attr.attr) {
                Attach(i);
            } else {
                Detach(i);
            }
        }

        if (attr.attr && m_vbuf) {
            auto num = m_vbuf->GetSizeInBytes() / attr.attr->GetStrideInBytes();
            m_max_array_index = std::max(num - 1, m_max_array_index);
        }
    }
}

void VertexBufferAttributes::Attach(int index)
{
    if (!m_vbuf) {
        return;
    }

    assert(index >= 0 && index < static_cast<int>(m_attrs.size()));

    auto& attr = m_attrs[index].attr;
    m_vbuf->Bind();

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(
        index,
        attr->GetNumOfComps(),
        TypeConverter::To(attr->GetCompDataType()),
        attr->GetNormalized(),
        attr->GetStrideInBytes(),
        (void*)(attr->GetOffsetInBytes())
    );
}

void VertexBufferAttributes::Detach(int index)
{
    glDisableVertexAttribArray(index);
}

}
}