#include "unirender/opengl/ShaderProgram.h"
#include "unirender/opengl/ShaderObject.h"
#include "unirender/opengl/Uniform.h"
#include "unirender/ShaderType.h"

#include <assert.h>

namespace
{

bool str_has_ending(const std::string& str, const std::string& end)
{
    if (str.length() >= end.length()) {
        return (0 == str.compare (str.length() - end.length(), end.length(), end));
    } else {
        return false;
    }
}

}

namespace ur
{
namespace opengl
{

ShaderProgram::ShaderProgram::ShaderProgram(const std::vector<unsigned int>& vs, 
                                            const std::vector<unsigned int>& fs)
{
    m_id = glCreateProgram();

    m_vs = std::make_shared<ShaderObject>(ShaderType::VertexShader, vs);
    m_vs->Attach(m_id);

    m_fs = std::make_shared<ShaderObject>(ShaderType::FragmentShader, fs);
    m_fs->Attach(m_id);

    glLinkProgram(m_id);
    CheckLinkStatus();

    InitVertexAttributes();
    InitUniforms();
}

ShaderProgram::ShaderProgram(const std::vector<unsigned int>& cs)
{
    m_id = glCreateProgram();

    m_cs = std::make_shared<ShaderObject>(ShaderType::ComputeShader, cs);
    m_cs->Attach(m_id);

    glLinkProgram(m_id);
    CheckLinkStatus();

    InitUniforms();
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_id);
}

void ShaderProgram::Bind() const
{
    glUseProgram(m_id);
}

bool ShaderProgram::CheckLinkStatus()
{
    GLint status;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    if (status == 0)
    {
        char buf[1024];
        GLint len;
        glGetProgramInfoLog(m_id, 1024, &len, buf);

        printf("link failed:%s\n", buf);

        return false;
    }
    else
    {
        return true;
    }
}

int ShaderProgram::GetComputeWorkGroupSize() const
{
    GLint threads[3];
    glGetProgramiv(m_id, GL_COMPUTE_WORK_GROUP_SIZE, threads);
    return threads[0];
}

int ShaderProgram::QueryTexSlot(const std::string& name) const
{
    for (int i = 0, n = m_tex_uniforms.size(); i < n; ++i) {
        if (m_tex_uniforms[i]->GetName() == name) {
            return i;
        }
    }
    return -1;
}

int ShaderProgram::QueryAttrLoc(const std::string& name) const
{
    for (auto& va : m_vertex_attributes) {
        if (va.name == name) {
            return va.location;
        }
    }
    return -1;
}

bool ShaderProgram::CheckStatus() const
{
    glValidateProgram(m_id);

    GLint status;
    glGetProgramiv(m_id, GL_VALIDATE_STATUS, &status);
    if (status == 0)
    {
        char buf[1024];
        GLint len;
        glGetProgramInfoLog(m_id, 1024, &len, buf);

        printf("shader error:%s\n", buf);

        return false;
    }
    else
    {
        return true;
    }
}

void ShaderProgram::InitVertexAttributes()
{
    GLint number_of_attributes;
    glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &number_of_attributes);

    GLint attribute_name_max_length;
    glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attribute_name_max_length);

    GLchar* name = new char[attribute_name_max_length];

    m_vertex_attributes.reserve(number_of_attributes);
    for (int i = 0; i < number_of_attributes; ++i)
    {
        GLsizei attribute_name_length;
        GLint attribute_length;
        GLenum attribute_type;
        glGetActiveAttrib(m_id, i, attribute_name_max_length,
            &attribute_name_length, &attribute_length, &attribute_type, name);

        int attribute_location = glGetAttribLocation(m_id, name);

        ShaderVertexAttribute dst;
        switch (attribute_type)
        {
        case GL_FLOAT:
            dst.type = ShaderVertexAttributeType::Float;
            break;
        case GL_FLOAT_VEC2:
            dst.type = ShaderVertexAttributeType::FloatVector2;
            break;
        case GL_FLOAT_VEC3:
            dst.type = ShaderVertexAttributeType::FloatVector3;
            break;
        case GL_FLOAT_VEC4:
            dst.type = ShaderVertexAttributeType::FloatVector4;
            break;
        case GL_FLOAT_MAT2:
            dst.type = ShaderVertexAttributeType::FloatMatrix22;
            break;
        case GL_FLOAT_MAT3:
            dst.type = ShaderVertexAttributeType::FloatMatrix33;
            break;
        case GL_FLOAT_MAT4:
            dst.type = ShaderVertexAttributeType::FloatMatrix44;
            break;
        case GL_INT:
            dst.type = ShaderVertexAttributeType::Int;
            break;
        case GL_INT_VEC2:
            dst.type = ShaderVertexAttributeType::IntVector2;
            break;
        case GL_INT_VEC3:
            dst.type = ShaderVertexAttributeType::IntVector3;
            break;
        case GL_INT_VEC4:
            dst.type = ShaderVertexAttributeType::IntVector4;
            break;
        case GL_UNSIGNED_INT:
            dst.type = ShaderVertexAttributeType::UInt;
            break;
        case GL_UNSIGNED_INT_VEC2:
            dst.type = ShaderVertexAttributeType::UIntVector2;
            break;
        case GL_UNSIGNED_INT_VEC3:
            dst.type = ShaderVertexAttributeType::UIntVector3;
            break;
        case GL_UNSIGNED_INT_VEC4:
            dst.type = ShaderVertexAttributeType::UIntVector4;
            break;
        default:
            assert(0);
        }
        dst.name     = name;
        dst.location = attribute_location;
        dst.length   = attribute_length;
        m_vertex_attributes.push_back(dst);
    }

    delete[] name;
}

void ShaderProgram::InitUniforms()
{
    GLint number_of_uniforms;
    glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &number_of_uniforms);

    GLint uniform_name_max_length;
    glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_name_max_length);

    GLchar* name_buf = new char[uniform_name_max_length];

    for (int i = 0; i < number_of_uniforms; ++i)
    {
        GLsizei uniform_name_length;
        GLint uniform_size;
        GLenum uniform_type;
        glGetActiveUniform(m_id, i, uniform_name_max_length,
            &uniform_name_length, &uniform_size, &uniform_type, name_buf);

		std::string name = name_buf;
		if (uniform_size > 1)
		{
			assert(str_has_ending(name, "[0]"));
			name = name.substr(0, name.size() - 3);
		}

        auto location = glGetUniformLocation(m_id, name.c_str());

        std::shared_ptr<ur::Uniform> uniform = nullptr;
        switch (uniform_type)
        {
        case GL_FLOAT:
            uniform = std::make_shared<Uniform<Float1>>(name, uniform_size, location);
            break;
        case GL_FLOAT_VEC2:
            uniform = std::make_shared<Uniform<Float2>>(name, uniform_size, location);
            break;
        case GL_FLOAT_VEC3:
            uniform = std::make_shared<Uniform<Float3>>(name, uniform_size, location);
            break;
        case GL_FLOAT_VEC4:
            uniform = std::make_shared<Uniform<Float4>>(name, uniform_size, location);
            break;
        case GL_BOOL:
            uniform = std::make_shared<Uniform<UInt1>>(name, uniform_size, location);
            break;
        case GL_INT:
            uniform = std::make_shared<Uniform<Int1>>(name, uniform_size, location);
            break;
        case GL_INT_VEC2:
            uniform = std::make_shared<Uniform<Int2>>(name, uniform_size, location);
            break;
        case GL_INT_VEC3:
            uniform = std::make_shared<Uniform<Int3>>(name, uniform_size, location);
            break;
        case GL_INT_VEC4:
            uniform = std::make_shared<Uniform<Int4>>(name, uniform_size, location);
            break;
        case GL_UNSIGNED_INT:
            uniform = std::make_shared<Uniform<UInt1>>(name, uniform_size, location);
            break;
        case GL_UNSIGNED_INT_VEC2:
            uniform = std::make_shared<Uniform<UInt2>>(name, uniform_size, location);
            break;
        case GL_UNSIGNED_INT_VEC3:
            uniform = std::make_shared<Uniform<UInt3>>(name, uniform_size, location);
            break;
        case GL_UNSIGNED_INT_VEC4:
            uniform = std::make_shared<Uniform<UInt4>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT2:
            uniform = std::make_shared<Uniform<Matrix22>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT3:
            uniform = std::make_shared<Uniform<Matrix33>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT4:
            uniform = std::make_shared<Uniform<Matrix44>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT2x3:
            uniform = std::make_shared<Uniform<Matrix23>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT2x4:
            uniform = std::make_shared<Uniform<Matrix32>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT3x2:
            uniform = std::make_shared<Uniform<Matrix24>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT3x4:
            uniform = std::make_shared<Uniform<Matrix42>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT4x2:
            uniform = std::make_shared<Uniform<Matrix34>>(name, uniform_size, location);
            break;
        case GL_FLOAT_MAT4x3:
            uniform = std::make_shared<Uniform<Matrix43>>(name, uniform_size, location);
            break;
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_SAMPLER_1D_ARRAY_EXT:
        case GL_SAMPLER_2D_ARRAY_EXT:
        case GL_SAMPLER_CUBE:
            uniform = std::make_shared<Uniform<Int1>>(name, uniform_size, location);
            m_tex_uniforms.push_back(uniform);
            break;
        default:
            assert(0);
        }

        AddUniform(name, uniform);
    }

    BindTextures();

    delete[] name_buf;
}

void ShaderProgram::BindTextures() const
{
    for (int i = 0, n = m_tex_uniforms.size(); i < n; ++i) {
        m_tex_uniforms[i]->SetValue(&i, 1);
    }
}

//void ShaderProgram::SetUniformValue(const std::string& name, UniformType type, const float* v, int n)
//{
//    auto itr = m_uniforms.find(name);
//    if (itr == m_uniforms.end()) {
//        return;
//    }
//
//    auto& u = itr->second;
//    assert(u.type == type);
//
//
//}

}
}