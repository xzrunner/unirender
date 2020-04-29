#pragma once

#include <memory>
#include <vector>

namespace ur
{

class VertexBuffer;
class VertexBufferAttribute;
class IndexBuffer;

class VertexArray
{
public:
    virtual ~VertexArray() {}

    virtual const std::shared_ptr<VertexBuffer> GetVertexBuffer() const = 0;
    virtual void SetVertexBuffer(const std::shared_ptr<ur::VertexBuffer>& buf) = 0;
    virtual void SetVertexBufferAttrs(const std::vector<std::shared_ptr<ur::VertexBufferAttribute>>& attrs) = 0;

    virtual const std::shared_ptr<IndexBuffer> GetIndexBuffer() const = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& buf) = 0;

}; // VertexArray

}