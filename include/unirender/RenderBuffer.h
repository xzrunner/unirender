#pragma once

#include "unirender/AttachmentType.h"

namespace ur
{

class RenderBuffer
{
public:
    virtual ~RenderBuffer() {}

    virtual void Bind(AttachmentType attach) const = 0;

}; // RenderBuffer

}