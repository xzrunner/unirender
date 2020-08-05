#include "unirender/VertexInputAttribute.h"

namespace ur
{

VertexInputAttribute::VertexInputAttribute(int loc, ComponentDataType comp_data_type,
                                             int num_of_comps, int offset_in_bytes, int stride_in_bytes)
    : m_loc(loc)
    , m_comp_data_type(comp_data_type)
    , m_num_of_comps(num_of_comps)
    , m_offset_in_bytes(offset_in_bytes)
    , m_stride_in_bytes(stride_in_bytes)
{
    if (comp_data_type == ComponentDataType::Int ||
        comp_data_type == ComponentDataType::UnsignedInt ||
        comp_data_type == ComponentDataType::Float) {
        m_normalized = false;
    } else {
        m_normalized = true;
    }
}

}