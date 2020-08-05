#pragma once

#include "unirender/VertexInputAttribute.h"

#include <vector>

namespace ur
{

class Device;
class VertexBuffer;

namespace opengl
{

struct VertexInputAttribute
{
    std::shared_ptr<ur::VertexInputAttribute> attr = nullptr;
    bool dirty = false;
};

class VertexInputAttributes
{
public:
    VertexInputAttributes(const Device& device);

    auto& GetAttrs() const { return m_attrs; }
    void SetAttrs(const std::vector<std::shared_ptr<ur::VertexInputAttribute>>& attrs);

    void Clean();

    int GetMaxArrayIndex() const { return m_max_array_index; }

    void SetVertexBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) { m_vbuf = buf; }

private:
    void Attach(int index);
    static void Detach(int index);

private:
    std::shared_ptr<ur::VertexBuffer> m_vbuf = nullptr;

    std::vector<VertexInputAttribute> m_attrs;

    int m_max_array_index = 0;

}; // VertexInputAttributes

}
}