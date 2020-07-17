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
    ShaderProgram(const std::vector<unsigned int>& vs, 
        const std::vector<unsigned int>& fs);
    ShaderProgram(const std::vector<unsigned int>& cs);
    virtual ~ShaderProgram();

    virtual void Bind() const override;
    virtual bool CheckStatus() const override;

    virtual int GetComputeWorkGroupSize() const override;

    virtual int QueryTexSlot(const std::string& name) const override;
    virtual int QueryAttrLoc(const std::string& name) const override;

private:
    bool CheckLinkStatus();

    void InitVertexAttributes();
    void InitUniforms();

    void BindTextures() const;

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