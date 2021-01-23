#pragma once

#include "unirender/ShaderType.h"
#include "unirender/opengl/opengl.h"

#include <string>
#include <vector>

namespace ur
{
namespace opengl
{

class ShaderObject
{
public:
    ShaderObject(ShaderType type,
        const std::vector<unsigned int>& spirv);
    ~ShaderObject();

    void Attach(GLuint prograpm);
    void Detach(GLuint prograpm);

    auto GetShaderType() const { return m_type; }

private:
    bool CheckCompileStatus(const std::string& source);

private:
    ShaderType m_type;

    GLuint m_id = 0;

}; // ShaderObject

}
}