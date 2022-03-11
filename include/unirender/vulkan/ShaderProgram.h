#pragma once

#include "unirender/ShaderProgram.h"

#include <vulkan/vulkan.h>

namespace ur
{

class Uniform;

namespace vulkan
{

class ShaderObject;
class LogicalDevice;

class ShaderProgram : public ur::ShaderProgram
{
public:
    ShaderProgram(const std::shared_ptr<LogicalDevice>& device, 
        const std::vector<unsigned int>& vs, const std::vector<unsigned int>& fs);
    ShaderProgram(const std::vector<unsigned int>& cs);
    virtual ~ShaderProgram();

    virtual void Bind() const override;
    virtual bool CheckStatus() const override;

    virtual void GetComputeWorkGroupSize(int& x, int& y, int& z) const override;

    virtual int QueryTexSlot(const std::string& name) const override;
    virtual int QueryAttrLoc(const std::string& name) const override;
    virtual int QueryImgSlot(const std::string& name) const override;

    virtual void BindSSBO(const std::string& name, int idx,
        const std::shared_ptr<StorageBuffer>& ssbo) const override {}

    virtual bool HasStage(ShaderType stage) const { return false; }

    auto& GetShaderStages() const { return m_shader_stages; }

private:
    std::shared_ptr<ShaderObject> m_vs = nullptr;
    std::shared_ptr<ShaderObject> m_fs = nullptr;

    VkPipelineShaderStageCreateInfo m_shader_stages[2];

}; // ShaderProgram

}
}