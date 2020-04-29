#pragma once

#include "unirender/Uniform.h"
#include "unirender/UniformValue.h"
#include "unirender/opengl/opengl.h"

#include <array>

#include <assert.h>

namespace ur
{
namespace opengl
{

template <typename T>
class Uniform : public ur::Uniform
{
public:
    Uniform(const std::string& name, int num, int location);

    virtual void Clean() override;

    virtual void SetValue(const int* value, int n = 1) override;
    virtual void SetValue(const float* value, int n = 1) override;

    void SetValue(T v, size_t index)
    {
        assert(index < m_vals.size());
        if (m_vals[i] != v) {
            m_vals[i] = v;
            m_dirty = true;
        }
    }

private:
    bool IsValSame(const int* value, int n) const;
    bool IsValSame(const float* value, int n) const;

private:
    std::vector<T> m_vals;

    bool m_dirty = false;

}; // Uniform

}
}

#include "unirender/opengl/Uniform.inl"