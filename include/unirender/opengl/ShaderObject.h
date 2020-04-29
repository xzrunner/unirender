#pragma once

#include "unirender/ShaderType.h"
#include "unirender/opengl/opengl.h"

#include <string>

namespace ur
{
namespace opengl
{

class ShaderObject
{
public:
    ShaderObject(ShaderType type, const std::string& source);
    ~ShaderObject();

    void Attach(GLuint prograpm);

private:
    bool CheckStatus(const std::string& source);

private:
    GLuint m_id = 0;

}; // ShaderObject

}
}