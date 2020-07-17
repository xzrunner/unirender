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

private:
    bool CheckCompileStatus(const std::string& source);

private:
    GLuint m_id = 0;

}; // ShaderObject

}
}