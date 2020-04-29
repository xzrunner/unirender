#pragma once

#include "unirender/ShaderProgram.h"
#include "unirender/ShaderVertexAttribute.h"
#include "unirender/Uniform.h"
#include "unirender/opengl/opengl.h"

#include <string>
#include <vector>

namespace ur
{

class Uniform;

namespace opengl
{

class ShaderObject;
class ShaderProgram : public ur::ShaderProgram
{
public:
    ShaderProgram(const std::string& vs, const std::string& fs,
        const std::string& gs = "", const std::vector<std::string>& attr_names = std::vector<std::string>());
    ShaderProgram(const std::string& cs);
    virtual ~ShaderProgram();

    virtual void Bind() const override;
    virtual bool CheckStatus() const override;

    virtual int GetComputeWorkGroupSize() const override;

    virtual int QueryTexSlot(const std::string& name) const override;

private:
    bool CheckStatus();

    void InitVertexAttributes();
    void InitUniforms();

    void BindTextures();

private:
    GLuint m_id = 0;

    std::shared_ptr<ShaderObject> m_vs = nullptr;
    std::shared_ptr<ShaderObject> m_gs = nullptr;
    std::shared_ptr<ShaderObject> m_fs = nullptr;

    std::shared_ptr<ShaderObject> m_cs = nullptr;

    std::vector<ShaderVertexAttribute> m_vertex_attributes;

    std::vector<std::shared_ptr<ur::Uniform>> m_tex_uniforms;

}; // ShaderProgram

}
}