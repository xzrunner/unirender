#pragma once

#include "unirender/BufferUsageHint.h"

#include <cstdint>

namespace ur
{

class ComputeBuffer
{
public:
    virtual ~ComputeBuffer() {}

    virtual void GetComputeBufferData(void* data, size_t size) const = 0;

}; // ComputeBuffer

}