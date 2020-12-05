#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/ShaderObject.h"
#include "unirender/ShaderType.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

ShaderProgram::ShaderProgram(const std::shared_ptr<LogicalDevice>& device, 
                             const std::vector<unsigned int>& vs,
                             const std::vector<unsigned int>& fs)
{
    m_vs = std::make_shared<ShaderObject>(device, ShaderType::VertexShader, vs);
    m_fs = std::make_shared<ShaderObject>(device, ShaderType::FragmentShader, fs);

    m_shader_stages[0] = m_vs->GetHandler();
    m_shader_stages[1] = m_fs->GetHandler();
}

ShaderProgram::ShaderProgram(const std::vector<unsigned int>& cs)
{
}

ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::Bind() const
{
}

bool ShaderProgram::CheckStatus() const
{
	return true;
}

void ShaderProgram::GetComputeWorkGroupSize(int& x, int& y, int& z) const
{
}

int ShaderProgram::QueryTexSlot(const std::string& name) const
{
    return -1;
}

int ShaderProgram::QueryAttrLoc(const std::string& name) const
{
    return -1;
}

}
}