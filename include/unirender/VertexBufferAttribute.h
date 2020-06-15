#pragma once

#include "unirender/ComponentDataType.h"

#include <memory>

namespace ur
{

class VertexBuffer;

class VertexBufferAttribute
{
public:
    VertexBufferAttribute(int loc, ComponentDataType comp_data_type, 
        int num_of_comps, int offset_in_bytes, int stride_in_bytes);

    auto GetLocation() const { return m_loc; }

    auto GetCompDataType() const { return m_comp_data_type; }
    auto GetNumOfComps() const { return m_num_of_comps; }

    auto GetNormalized() const { return m_normalized; }

    auto GetOffsetInBytes() const { return m_offset_in_bytes; }
    auto GetStrideInBytes() const { return m_stride_in_bytes; }

private:
    int m_loc = -1;

    ComponentDataType m_comp_data_type;
    int m_num_of_comps = 0;

    bool m_normalized = false;

    int m_offset_in_bytes = 0;
    int m_stride_in_bytes = 0;

}; // VertexBufferAttribute

}