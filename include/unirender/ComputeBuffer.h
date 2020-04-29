#pragma once

#include "unirender/BufferUsageHint.h"

#include <cstdint>
#include <vector>

namespace ur
{

class ComputeBuffer
{
public:
    virtual ~ComputeBuffer() {}

    virtual void GetComputeBufferData(std::vector<int>& result) const = 0;

}; // ComputeBuffer

}