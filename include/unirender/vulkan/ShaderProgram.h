#pragma once

#include "unirender/ShaderProgram.h"

#include <vulkan/vulkan.h>

namespace ur
{

class Uniform;

namespace vulkan
{

class ShaderObject;
class ShaderProgram : public ur::ShaderProgram
{
public:
    ShaderProgram(VkDevice dev, const std::string& vs, const std::string& fs,
        const std::string& gs = "", const std::vector<std::string>& attr_names = std::vector<std::string>());
    ShaderProgram(const std::string& cs);
    virtual ~ShaderProgram();

    virtual void Bind() const override;
    virtual bool CheckStatus() const override;

    virtual int GetComputeWorkGroupSize() const override;

    virtual int QueryTexSlot(const std::string& name) const override;
    virtual int QueryAttrLoc(const std::string& name) const override;

    auto& GetShaderStages() const { return m_shader_stages; }

private:
    std::shared_ptr<ShaderObject> m_vs = nullptr;
    std::shared_ptr<ShaderObject> m_fs = nullptr;

    VkPipelineShaderStageCreateInfo m_shader_stages[2];

}; // ShaderProgram

}
}