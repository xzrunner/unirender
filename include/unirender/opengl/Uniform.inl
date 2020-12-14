#pragma once

#define FLOAT_UNIFORM_IMPL(Type, Size)                              \
template <>                                                         \
void Uniform<Type>::SetValue(const int* value, int n)               \
{                                                                   \
    assert(0);                                                      \
}                                                                   \
                                                                    \
template <>                                                         \
void Uniform<Type>::SetValue(const float* value, int n)             \
{                                                                   \
    assert(value && n > 0);                                         \
    if (IsValSame(value, n)) {                                      \
        return;                                                     \
    }                                                               \
                                                                    \
    const size_t size = Size;                                       \
    const size_t num = n / size;                                    \
    if (num != m_vals.size()) {                                     \
        m_vals.resize(num);                                         \
        m_num = num;                                                \
    }                                                               \
    for (size_t i = 0; i < num; ++i) {                              \
        for (size_t j = 0; j < size; ++j) {                         \
            m_vals[i].v[j] = value[i * size + j];                   \
        }                                                           \
    }                                                               \
    m_dirty = true;                                                 \
}                                                                   \
                                                                    \
template <>                                                         \
bool Uniform<Type>::IsValSame(const int* value, int n) const        \
{                                                                   \
    return false;                                                   \
}                                                                   \
                                                                    \
template <>                                                         \
bool Uniform<Type>::IsValSame(const float* value, int n) const      \
{                                                                   \
    if (m_vals.empty()) {                                           \
        return false;                                               \
    }                                                               \
    assert(value || n > 0);                                         \
                                                                    \
    const size_t size = Size;                                       \
    const size_t num = n / size;                                    \
    if (num != m_num) {                                             \
        return false;                                               \
    }                                                               \
                                                                    \
    assert(num == m_num);                                           \
    for (size_t i = 0; i < num; ++i) {                              \
        for (size_t j = 0; j < size; ++j) {                         \
            if (m_vals[i].v[j] != value[i * size + j]) {            \
                return false;                                       \
            }                                                       \
        }                                                           \
    }                                                               \
                                                                    \
    return true;                                                    \
}

#define INT_UNIFORM_IMPL(Type, Size)                                \
template <>                                                         \
void Uniform<Type>::SetValue(const int* value, int n)               \
{                                                                   \
    assert(value && n > 0);                                         \
    if (IsValSame(value, n)) {                                      \
        return;                                                     \
    }                                                               \
                                                                    \
    const size_t size = Size;                                       \
    const size_t num = n / size;                                    \
    if (num != m_vals.size()) {                                     \
        m_vals.resize(num);                                         \
        m_num = num;                                                \
    }                                                               \
    for (size_t i = 0; i < num; ++i) {                              \
        for (size_t j = 0; j < size; ++j) {                         \
            m_vals[i].v[j] = value[i * size + j];                   \
        }                                                           \
    }                                                               \
    m_dirty = true;                                                 \
}                                                                   \
                                                                    \
template <>                                                         \
void Uniform<Type>::SetValue(const float* value, int n)             \
{                                                                   \
    assert(value && n > 0);                                         \
    if (IsValSame(value, n)) {                                      \
        return;                                                     \
    }                                                               \
                                                                    \
    const size_t size = Size;                                       \
    const size_t num = n / size;                                    \
    if (num != m_vals.size()) {                                     \
        m_vals.resize(num);                                         \
        m_num = num;                                                \
    }                                                               \
    for (size_t i = 0; i < num; ++i) {                              \
        for (size_t j = 0; j < size; ++j) {                         \
            m_vals[i].v[j] = static_cast<int>(value[i * size + j] + 0.5f); \
        }                                                           \
    }                                                               \
    m_dirty = true;                                                 \
}                                                                   \
                                                                    \
template <>                                                         \
bool Uniform<Type>::IsValSame(const int* value, int n) const        \
{                                                                   \
    assert(value || n > 0);                                         \
    if (m_vals.empty()) {                                           \
        return false;                                               \
    }                                                               \
                                                                    \
    const size_t size = Size;                                       \
    const size_t num = n / size;                                    \
    if (num != m_num) {                                             \
        return false;                                               \
    }                                                               \
                                                                    \
    assert(num == m_num);                                           \
    for (size_t i = 0; i < num; ++i) {                              \
        for (size_t j = 0; j < size; ++j) {                         \
            if (m_vals[i].v[j] != value[i * size + j]) {            \
                return false;                                       \
            }                                                       \
        }                                                           \
    }                                                               \
                                                                    \
    return true;                                                    \
}                                                                   \
                                                                    \
template <>                                                         \
bool Uniform<Type>::IsValSame(const float* value, int n) const      \
{                                                                   \
    assert(value || n > 0);                                         \
    if (m_vals.empty()) {                                           \
        return false;                                               \
    }                                                               \
                                                                    \
    const size_t size = Size;                                       \
    const size_t num = n / size;                                    \
    if (num != m_num) {                                             \
        return false;                                               \
    }                                                               \
                                                                    \
    assert(num == m_num);                                           \
    for (size_t i = 0; i < num; ++i) {                              \
        for (size_t j = 0; j < size; ++j) {                         \
            if (m_vals[i].v[j] != static_cast<float>(value[i * size + j] + 0.5f)) { \
                return false;                                       \
            }                                                       \
        }                                                           \
    }                                                               \
                                                                    \
    return true;                                                    \
}

namespace ur
{
namespace opengl
{

// Float1

template <>
Uniform<Float1>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Float1, num, location)
{
}

template <>
void Uniform<Float1>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform1f(m_location, m_vals[0].x);
    } else {
        glUniform1fv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Float1, 1)

// Float2

template <>
Uniform<Float2>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Float2, num, location)
{
}

template <>
void Uniform<Float2>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform2f(m_location, m_vals[0].x, m_vals[0].y);
    } else {
        glUniform2fv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Float2, 2)

// Float3

template <>
Uniform<Float3>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Float3, num, location)
{
}

template <>
void Uniform<Float3>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform3f(m_location, m_vals[0].x, m_vals[0].y, m_vals[0].z);
    } else {
        glUniform3fv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Float3, 3)

// Float4

template <>
Uniform<Float4>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Float4, num, location)
{
}

template <>
void Uniform<Float4>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform4f(m_location, m_vals[0].x, m_vals[0].y, m_vals[0].z, m_vals[0].w);
    } else {
        glUniform4fv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Float4, 4)

// Int1

template <>
Uniform<Int1>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Int1, num, location)
{
}

template <>
void Uniform<Int1>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform1i(m_location, m_vals[0].x);
    } else {
        glUniform1iv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

INT_UNIFORM_IMPL(Int1, 1)

// Int2

template <>
Uniform<Int2>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Int2, num, location)
{
}

template <>
void Uniform<Int2>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform2i(m_location, m_vals[0].x, m_vals[0].y);
    } else {
        glUniform2iv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

INT_UNIFORM_IMPL(Int2, 2)

// Int3

template <>
Uniform<Int3>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Int3, num, location)
{
}

template <>
void Uniform<Int3>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform3i(m_location, m_vals[0].x, m_vals[0].y, m_vals[0].z);
    } else {
        glUniform3iv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

INT_UNIFORM_IMPL(Int3, 3)

// Int4

template <>
Uniform<Int4>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Int4, num, location)
{
}

template <>
void Uniform<Int4>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform4i(m_location, m_vals[0].x, m_vals[0].y, m_vals[0].z, m_vals[0].w);
    } else {
        glUniform4iv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

INT_UNIFORM_IMPL(Int4, 4)

// UInt1

template <>
Uniform<UInt1>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::UInt1, num, location)
{
}

template <>
void Uniform<UInt1>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform1ui(m_location, m_vals[0].x);
    } else {
        glUniform1uiv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

INT_UNIFORM_IMPL(UInt1, 1)

// UInt2

template <>
Uniform<UInt2>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::UInt2, num, location)
{
}

template <>
void Uniform<UInt2>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform2ui(m_location, m_vals[0].x, m_vals[0].y);
    } else {
        glUniform2uiv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

INT_UNIFORM_IMPL(UInt2, 2)

// UInt3

template <>
Uniform<UInt3>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::UInt3, num, location)
{
}

template <>
void Uniform<UInt3>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform3ui(m_location, m_vals[0].x, m_vals[0].y, m_vals[0].z);
    } else {
        glUniform3uiv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

INT_UNIFORM_IMPL(UInt3, 3)

// UInt4

template <>
Uniform<UInt4>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::UInt4, num, location)
{
}

template <>
void Uniform<UInt4>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    if (m_num == 1) {
        glUniform4ui(m_location, m_vals[0].x, m_vals[0].y, m_vals[0].z, m_vals[0].w);
    } else {
        glUniform4uiv(m_location, m_num, &m_vals[0].x);
    }

    m_dirty = false;
}

INT_UNIFORM_IMPL(UInt4, 4)

// Matrix22

template <>
Uniform<Matrix22>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix22, num, location)
{
}

template <>
void Uniform<Matrix22>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix2fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix22, 2 * 2)

// Matrix33

template <>
Uniform<Matrix33>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix33, num, location)
{
}

template <>
void Uniform<Matrix33>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix3fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix33, 3 * 3)

// Matrix44

template <>
Uniform<Matrix44>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix44, num, location)
{
}

template <>
void Uniform<Matrix44>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix4fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix44, 4 * 4)

// Matrix23

template <>
Uniform<Matrix23>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix23, num, location)
{
}

template <>
void Uniform<Matrix23>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix2x3fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix23, 2 * 3)

// Matrix32

template <>
Uniform<Matrix32>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix32, num, location)
{
}

template <>
void Uniform<Matrix32>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix3x2fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix32, 3 * 2)

// Matrix24

template <>
Uniform<Matrix24>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix24, num, location)
{
}

template <>
void Uniform<Matrix24>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix2x4fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix24, 2 * 4)

// Matrix42

template <>
Uniform<Matrix42>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix42, num, location)
{
}

template <>
void Uniform<Matrix42>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix4x2fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix42, 4 * 2)

// Matrix34

template <>
Uniform<Matrix34>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix34, num, location)
{
}

template <>
void Uniform<Matrix34>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix3x4fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix34, 3 * 4)

// Matrix43

template <>
Uniform<Matrix43>::Uniform(const std::string& name, int num, int location)
    : ur::Uniform(name, UniformType::Matrix43, num, location)
{
}

template <>
void Uniform<Matrix43>::Clean()
{
    if (!m_dirty || m_vals.empty()) {
        return;
    }

    assert(m_num > 0);
    glUniformMatrix4x3fv(m_location, m_num, GL_FALSE, &m_vals[0].m[0][0]);

    m_dirty = false;
}

FLOAT_UNIFORM_IMPL(Matrix43, 4 * 3)

}
}