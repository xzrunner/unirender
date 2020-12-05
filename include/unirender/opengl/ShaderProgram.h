#pragma once

#include "unirender/ShaderProgram.h"
#include "unirender/ShaderVertexAttribute.h"
#include "unirender/Uniform.h"
#include "unirender/opengl/opengl.h"

#include <string>
#include <vector>
#include <iostream>

namespace ur
{

class Uniform;

namespace opengl
{

class ShaderObject;
class ShaderProgram : public ur::ShaderProgram
{
public:
    ShaderProgram(
        const std::vector<unsigned int>& vs,
        const std::vector<unsigned int>& fs,
        const std::vector<unsigned int>& tcs,
        const std::vector<unsigned int>& tes,
        std::ostream& out = std::cerr
    );
    ShaderProgram(const std::vector<unsigned int>& cs);
    virtual ~ShaderProgram();

    virtual void Bind() const override;
    virtual bool CheckStatus() const override;

    virtual void GetComputeWorkGroupSize(int& x, int& y, int& z) const override;

    virtual int QueryTexSlot(const std::string& name) const override;
    virtual int QueryAttrLoc(const std::string& name) const override;

    virtual bool HasStage(ShaderType stage) const;

private:
    bool CheckLinkStatus(std::ostream& out = std::cerr);

    void InitVertexAttributes();
    void InitUniforms();

    void BindTextures() const;

private:
    GLuint m_id = 0;

    std::shared_ptr<ShaderObject> m_vs  = nullptr;
    std::shared_ptr<ShaderObject> m_tcs = nullptr;
    std::shared_ptr<ShaderObject> m_tes = nullptr;
    std::shared_ptr<ShaderObject> m_gs  = nullptr;
    std::shared_ptr<ShaderObject> m_fs  = nullptr;

    std::shared_ptr<ShaderObject> m_cs = nullptr;

    std::vector<ShaderVertexAttribute> m_vertex_attributes;

    std::vector<std::shared_ptr<ur::Uniform>> m_tex_uniforms;

}; // ShaderProgram

}
}