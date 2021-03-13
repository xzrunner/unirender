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
        const std::vector<unsigned int>& gs,
        std::ostream& out = std::cerr
    );
    ShaderProgram(const std::vector<unsigned int>& cs);
    virtual ~ShaderProgram();

    virtual void Bind() const override;
    virtual bool CheckStatus() const override;

    virtual void GetComputeWorkGroupSize(int& x, int& y, int& z) const override;

    virtual int QueryTexSlot(const std::string& name) const override;
    virtual int QueryAttrLoc(const std::string& name) const override;
    virtual int QueryImgSlot(const std::string& name) const override;

    virtual bool HasStage(ShaderType stage) const;

private:
    bool CheckLinkStatus(std::ostream& out = std::cerr);

    void InitVertexAttributes();
    void InitUniforms();

    void BindTextures() const;

    void AddUniform(const std::string& name, GLenum type, GLint size);

    static int GetUnitAvaliable(const std::vector<std::shared_ptr<ur::Uniform>>& unifs, int unit);

private:
    GLuint m_id = 0;

    std::vector<std::shared_ptr<ShaderObject>> m_shaders;

    std::vector<ShaderVertexAttribute> m_vertex_attributes;

    std::vector<std::shared_ptr<ur::Uniform>> m_tex_uniforms;
    std::vector<std::shared_ptr<ur::Uniform>> m_img_uniforms;

}; // ShaderProgram

}
}