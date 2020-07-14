#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/ShaderObject.h"
#include "unirender/ShaderType.h"

#include <assert.h>

namespace ur
{
namespace vulkan
{

ShaderProgram::ShaderProgram(VkDevice dev, const uint32_t* vs, size_t vs_size,
                             const uint32_t* fs, size_t fs_size)
{
    m_vs = std::make_shared<ShaderObject>(dev, ShaderType::VertexShader, vs, vs_size);
    m_fs = std::make_shared<ShaderObject>(dev, ShaderType::FragmentShader, fs, fs_size);

    m_shader_stages[0] = m_vs->GetHandler();
    m_shader_stages[1] = m_fs->GetHandler();
}

ShaderProgram::ShaderProgram(const std::string& cs)
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

int ShaderProgram::GetComputeWorkGroupSize() const
{
	return 0;
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