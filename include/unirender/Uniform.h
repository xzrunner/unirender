#pragma once

#include "unirender/UniformType.h"

#include <string>

namespace ur
{

class Uniform
{
public:
    Uniform(const std::string& name, UniformType type, int num, int location)
        : m_name(name)
        , m_type(type)
        , m_num(num)
        , m_location(location)
    {
    }

    virtual void Clean() = 0;

    virtual void SetValue(const int* value, int n = 1) = 0;
    virtual void SetValue(const float* value, int n = 1) = 0;

    auto& GetName() const { return m_name; }
    auto GetType() const { return m_type; }

protected:
    std::string m_name;
    UniformType m_type;

    int m_num      = 0;
    int m_location = 0;

}; // Uniform

}