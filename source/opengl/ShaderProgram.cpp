﻿#include "unirender/opengl/ShaderProgram.h"
#include "unirender/opengl/ShaderObject.h"
#include "unirender/opengl/Uniform.h"
#include "unirender/opengl/StorageBuffer.h"
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
                                            const std::vector<unsigned int>& fs,
                                            const std::vector<unsigned int>& tcs,
                                            const std::vector<unsigned int>& tes,
                                            const std::vector<unsigned int>& gs,
                                            std::ostream& out)
{
    m_id = glCreateProgram();

    if (!vs.empty()) {
        m_shaders.push_back(std::make_shared<ShaderObject>(ShaderType::VertexShader, vs));
    }
    if (!fs.empty()) {
        m_shaders.push_back(std::make_shared<ShaderObject>(ShaderType::FragmentShader, fs));
    }
    if (!tcs.empty()) {
        m_shaders.push_back(std::make_shared<ShaderObject>(ShaderType::TessCtrlShader, tcs));
    }
    if (!tes.empty()) {
        m_shaders.push_back(std::make_shared<ShaderObject>(ShaderType::TessEvalShader, tes));
    }
    if (!gs.empty()) {
        m_shaders.push_back(std::make_shared<ShaderObject>(ShaderType::GeometryShader, gs));
    }
    for (auto& shader : m_shaders) {
        shader->Attach(m_id);
    }

    glLinkProgram(m_id);
    CheckLinkStatus(out);
    PrepareObjects();
}

ShaderProgram::ShaderProgram(const std::vector<unsigned int>& cs)
{
    m_id = glCreateProgram();

    m_shaders.push_back(std::make_shared<ShaderObject>(ShaderType::ComputeShader, cs));
    for (auto& shader : m_shaders) {
        shader->Attach(m_id);
    }

    glLinkProgram(m_id);
    CheckLinkStatus();
    PrepareObjects();
}

ShaderProgram::ShaderProgram(const std::string& cs)
{
    m_id = glCreateProgram();

    m_shaders.push_back(std::make_shared<ShaderObject>(ShaderType::ComputeShader, cs));
    for (auto& shader : m_shaders) {
        shader->Attach(m_id);
    }

    glLinkProgram(m_id);
    CheckLinkStatus();
    PrepareObjects();
}

ShaderProgram::~ShaderProgram()
{
    for (auto& shader : m_shaders) {
        shader->Detach(m_id);
    }

    glDeleteProgram(m_id);
}

void ShaderProgram::Bind() const
{
    glUseProgram(m_id);
}

bool ShaderProgram::CheckLinkStatus(std::ostream& out)
{
    GLint status;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    if (status == 0)
    {
        char buf[1024];
        GLint len;
        glGetProgramInfoLog(m_id, 1024, &len, buf);

        out << "link failed:\n" << buf << "\n";

        return false;
    }
    else
    {
        return true;
    }
}

void ShaderProgram::GetComputeWorkGroupSize(int& x, int& y, int& z) const
{
    GLint threads[3];
    glGetProgramiv(m_id, GL_COMPUTE_WORK_GROUP_SIZE, threads);
    x = threads[0];
    y = threads[1];
    z = threads[2];
}

int ShaderProgram::QueryTexSlot(const std::string& name) const
{
    for (auto& unif : m_tex_uniforms) {
        if (unif->GetName() == name) {
            return unif->GetUnit();
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

int ShaderProgram::QueryImgSlot(const std::string& name) const
{
    for (auto& unif : m_img_uniforms) {
        if (unif->GetName() == name) {
            return unif->GetUnit();
        }
    }
    return -1;
}

void ShaderProgram::BindSSBO(const std::string& name, int idx, 
                             const std::shared_ptr<ur::StorageBuffer>& ssbo) const
{
    std::static_pointer_cast<ur::opengl::StorageBuffer>(ssbo)->BindIndex(idx);
    GLuint loc = glGetProgramResourceIndex(m_id, GL_SHADER_STORAGE_BLOCK, name.c_str());
    glShaderStorageBlockBinding(m_id, loc, idx);
}

bool ShaderProgram::HasStage(ShaderType stage) const
{
    for (auto& shader : m_shaders) {
        if (shader->GetShaderType() == stage) {
            return true;
        }
    }
    return false;
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

        // Intel HD/Iris Graphics GPU's bug
        if (std::string(buf).find("Validation Error: A sampler points to a texture unit used by fixed function with an incompatible target.") != std::string::npos) {
            return true;
        }

        if (len > 0) {
            printf("shader error:[%s]\n", buf);
        }

        return false;
    }
    else
    {
        return true;
    }
}

void ShaderProgram::PrepareObjects()
{
    InitVertexAttributes();
    InitUniforms();
    //InitResources();
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
            for (int i = 0; i < uniform_size; ++i) {
                AddUniform(name + "[" + std::to_string(i) + "]", uniform_type, 1);
            }
		}
        else
        {
            AddUniform(name, uniform_type, 1);
        }
    }

    BindTextures();

    delete[] name_buf;
}

void ShaderProgram::InitResources()
{
    GLint no_of, ssbo_max_len, var_max_len;
    glGetProgramInterfaceiv(m_id, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &no_of);
    glGetProgramInterfaceiv(m_id, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &ssbo_max_len);
    glGetProgramInterfaceiv(m_id, GL_BUFFER_VARIABLE, GL_MAX_NAME_LENGTH, &var_max_len);

    std::vector< GLchar >name(ssbo_max_len);
    for (int i_resource = 0; i_resource < no_of; i_resource++) 
    {
        // get name of the shader storage block
        GLsizei strLength;
        glGetProgramResourceName(m_id, GL_SHADER_STORAGE_BLOCK, i_resource, ssbo_max_len, &strLength, name.data());

        // get resource index of the shader storage block
        GLint resInx = glGetProgramResourceIndex(m_id, GL_SHADER_STORAGE_BLOCK, name.data());

        // get number of the buffer variables in the shader storage block
        GLenum prop = GL_NUM_ACTIVE_VARIABLES;
        GLint num_var;
        glGetProgramResourceiv(m_id, GL_SHADER_STORAGE_BLOCK, resInx, 1, &prop, 1, nullptr, &num_var);

        // get resource indices of the buffer variables
        std::vector<GLint> vars(num_var);
        prop = GL_ACTIVE_VARIABLES;
        glGetProgramResourceiv(m_id, GL_SHADER_STORAGE_BLOCK, resInx, 1, &prop, (GLsizei)vars.size(), nullptr, vars.data());

        std::vector<GLint> offsets(num_var);
        std::vector<std::string> var_names(num_var);
        for (GLint i = 0; i < num_var; i++) 
        {
            // get offset of buffer variable relative to SSBO
            GLenum prop = GL_OFFSET;
            glGetProgramResourceiv(m_id, GL_BUFFER_VARIABLE, vars[i], 1, &prop, (GLsizei)offsets.size(), nullptr, &offsets[i]);

            // get name of buffer variable
            std::vector<GLchar>var_name(var_max_len);
            GLsizei strLength;
            glGetProgramResourceName(m_id, GL_BUFFER_VARIABLE, vars[i], var_max_len, &strLength, var_name.data());
            var_names[i] = var_name.data();
        }


    }
}

void ShaderProgram::BindTextures() const
{
    for (auto& unif : m_tex_uniforms)
    {
        const int unit = unif->GetUnit();
        assert(unit >= 0);
        unif->SetValue(&unit, 1);
    }
}

void ShaderProgram::AddUniform(const std::string& name, GLenum type, GLint size)
{
    auto location = glGetUniformLocation(m_id, name.c_str());

    std::shared_ptr<ur::Uniform> uniform = nullptr;
    switch (type)
    {
    case GL_FLOAT:
        uniform = std::make_shared<Uniform<Float1>>(name, size, location);
        break;
    case GL_FLOAT_VEC2:
        uniform = std::make_shared<Uniform<Float2>>(name, size, location);
        break;
    case GL_FLOAT_VEC3:
        uniform = std::make_shared<Uniform<Float3>>(name, size, location);
        break;
    case GL_FLOAT_VEC4:
        uniform = std::make_shared<Uniform<Float4>>(name, size, location);
        break;
    case GL_BOOL:
        uniform = std::make_shared<Uniform<UInt1>>(name, size, location);
        break;
    case GL_INT:
        uniform = std::make_shared<Uniform<Int1>>(name, size, location);
        break;
    case GL_INT_VEC2:
        uniform = std::make_shared<Uniform<Int2>>(name, size, location);
        break;
    case GL_INT_VEC3:
        uniform = std::make_shared<Uniform<Int3>>(name, size, location);
        break;
    case GL_INT_VEC4:
        uniform = std::make_shared<Uniform<Int4>>(name, size, location);
        break;
    case GL_UNSIGNED_INT:
        uniform = std::make_shared<Uniform<UInt1>>(name, size, location);
        break;
    case GL_UNSIGNED_INT_VEC2:
        uniform = std::make_shared<Uniform<UInt2>>(name, size, location);
        break;
    case GL_UNSIGNED_INT_VEC3:
        uniform = std::make_shared<Uniform<UInt3>>(name, size, location);
        break;
    case GL_UNSIGNED_INT_VEC4:
        uniform = std::make_shared<Uniform<UInt4>>(name, size, location);
        break;
    case GL_FLOAT_MAT2:
        uniform = std::make_shared<Uniform<Matrix22>>(name, size, location);
        break;
    case GL_FLOAT_MAT3:
        uniform = std::make_shared<Uniform<Matrix33>>(name, size, location);
        break;
    case GL_FLOAT_MAT4:
        uniform = std::make_shared<Uniform<Matrix44>>(name, size, location);
        break;
    case GL_FLOAT_MAT2x3:
        uniform = std::make_shared<Uniform<Matrix23>>(name, size, location);
        break;
    case GL_FLOAT_MAT2x4:
        uniform = std::make_shared<Uniform<Matrix32>>(name, size, location);
        break;
    case GL_FLOAT_MAT3x2:
        uniform = std::make_shared<Uniform<Matrix24>>(name, size, location);
        break;
    case GL_FLOAT_MAT3x4:
        uniform = std::make_shared<Uniform<Matrix42>>(name, size, location);
        break;
    case GL_FLOAT_MAT4x2:
        uniform = std::make_shared<Uniform<Matrix34>>(name, size, location);
        break;
    case GL_FLOAT_MAT4x3:
        uniform = std::make_shared<Uniform<Matrix43>>(name, size, location);
        break;
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_3D:
    case GL_SAMPLER_1D_ARRAY:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_BUFFER:
    case GL_INT_SAMPLER_BUFFER:
    {
        uniform = std::make_shared<Uniform<Int1>>(name, size, location);

        GLint unit = -1;
        glGetUniformiv(m_id, location, &unit);
        uniform->SetUnit(GetUnitAvaliable(m_tex_uniforms, unit));

        m_tex_uniforms.push_back(uniform);
    }
        break;
    case GL_IMAGE_2D:
    {
        uniform = std::make_shared<Uniform<Int1>>(name, size, location);

        GLint unit = -1;
        glGetUniformiv(m_id, location, &unit);
        uniform->SetUnit(GetUnitAvaliable(m_img_uniforms, unit));

        m_img_uniforms.push_back(uniform);
    }
        break;
    default:
        assert(0);
    }

    if (uniform) {
        ur::ShaderProgram::AddUniform(name, uniform);
    }
}

int ShaderProgram::GetUnitAvaliable(const std::vector<std::shared_ptr<ur::Uniform>>& unifs, int unit)
{
    auto is_exist = [&] (int i) {
        for (auto& u : unifs) {
            if (u->GetUnit() == i) {
                return true;
            }
        }
        return false;
    };

    if (!is_exist(unit)) {
        return unit;
    }

    int i = 0;
    while (true)
    {
        if (!is_exist(i)) {
            return i;
        }
        ++i;
    }

    assert(0);
    return 0;
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