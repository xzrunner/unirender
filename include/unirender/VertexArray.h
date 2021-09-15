#pragma once

#include <memory>
#include <vector>

namespace ur
{

class VertexBuffer;
class VertexInputAttribute;
class IndexBuffer;

class VertexArray
{
public:
    virtual ~VertexArray() {}

    virtual const std::shared_ptr<VertexBuffer> GetVertexBuffer() const = 0;
    virtual void SetVertexBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) = 0;
    virtual std::vector<std::shared_ptr<ur::VertexInputAttribute>> GetVertexBufferAttrs() const = 0;
    virtual void SetVertexBufferAttrs(const std::vector<std::shared_ptr<ur::VertexInputAttribute>>& attrs) = 0;

    virtual const std::shared_ptr<IndexBuffer> GetIndexBuffer() const = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& buf) = 0;

    virtual void SetInstanceBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) = 0;

}; // VertexArray

}