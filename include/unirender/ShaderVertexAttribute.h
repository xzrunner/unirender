#pragma once

#include <string>

namespace ur
{

enum class ShaderVertexAttributeType
{
    Float,
    FloatVector2,
    FloatVector3,
    FloatVector4,
    FloatMatrix22,
    FloatMatrix33,
    FloatMatrix44,
    Int,
    IntVector2,
    IntVector3,
    IntVector4
};

struct ShaderVertexAttribute
{
    ShaderVertexAttributeType type;
    std::string name;
    int location = 0;
    int length   = 0;
};

}