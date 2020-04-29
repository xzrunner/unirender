#pragma once

#include "unirender/IndexBuffer.h"
#include "unirender/ComponentDataType.h"

#include <assert.h>

namespace ur
{

class VertexArraySizes
{
public:
    static int SizeOf(IndexBufferDataType type)
    {
        switch (type)
        {
        case IndexBufferDataType::UnsignedShort:
            return sizeof(unsigned short);
        case IndexBufferDataType::UnsignedInt:
            return sizeof(unsigned int);
        default:
            assert(0);
            return 0;
        }
    }

    static int SizeOf(ComponentDataType type)
    {
        switch (type)
        {
        case ComponentDataType::Byte:
        case ComponentDataType::UnsignedByte:
            return sizeof(char);
        case ComponentDataType::Short:
            return sizeof(short);
        case ComponentDataType::UnsignedShort:
            return sizeof(unsigned short);
        case ComponentDataType::Int:
            return sizeof(int);
        case ComponentDataType::UnsignedInt:
            return sizeof(unsigned int);
        case ComponentDataType::Float:
            return sizeof(float);
        case ComponentDataType::HalfFloat:
            return sizeof(float) / 2;
        default:
            assert(0);
            return 0;
        }
    }

}; // VertexArraySizes

}