#pragma once

namespace ur
{

class RenderBuffer
{
public:
    virtual ~RenderBuffer() {}

    virtual void Bind() const = 0;

}; // RenderBuffer

}