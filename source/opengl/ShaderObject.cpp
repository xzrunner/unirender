#include "unirender/opengl/ShaderObject.h"
#include "unirender/opengl/TypeConverter.h"
#include "unirender/Adaptor.h"

#include <shadertrans/ShaderTrans.h>

namespace ur
{
namespace opengl
{

ShaderObject::ShaderObject(ShaderType type, const std::vector<unsigned int>& spirv)
    : m_type(type)
{
    std::string glsl;
    auto stage = Adaptor::ToShaderTransStage(type);
    shadertrans::ShaderTrans::SpirV2GLSL(stage, spirv, glsl);

    m_id = glCreateShader(TypeConverter::To(type));

    const char* c = glsl.c_str();
    glShaderSource(m_id, 1, &c, NULL);

    glCompileShader(m_id);

    CheckCompileStatus(glsl);
}

ShaderObject::~ShaderObject()
{
    glDeleteShader(m_id);
}

void ShaderObject::Attach(GLuint prograpm)
{
    if (m_id > 0) {
        glAttachShader(prograpm, m_id);
    }
}

void ShaderObject::Detach(GLuint prograpm)
{
    if (m_id > 0) {
        glDetachShader(prograpm, m_id);
    }
}

bool ShaderObject::CheckCompileStatus(const std::string& source)
{
    GLint status;
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        char buf[1024];
        GLint len;
        glGetShaderInfoLog(m_id, 1024, &len, buf);

        printf("compile failed:\n %s, source:\n %s\n",
            buf, source.c_str());
        glDeleteShader(m_id);
        m_id = 0;

        return false;
    }
    else
    {
        return true;
    }
}

}
}