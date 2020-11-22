#include "unirender/Adaptor.h"

#include <assert.h>

namespace ur
{

shadertrans::ShaderStage 
Adaptor::ToShaderTransStage(ShaderType type)
{
	switch (type)
	{
	case ShaderType::VertexShader:
		return shadertrans::ShaderStage::VertexShader;
	case ShaderType::TessCtrlShader:
		return shadertrans::ShaderStage::TessCtrlShader;
	case ShaderType::TessEvalShader:
		return shadertrans::ShaderStage::TessEvalShader;
	case ShaderType::GeometryShader:
		return shadertrans::ShaderStage::GeometryShader;
	case ShaderType::FragmentShader:
		return shadertrans::ShaderStage::PixelShader;
	case ShaderType::ComputeShader:
		return shadertrans::ShaderStage::ComputeShader;
	default:
		assert(0);
		return shadertrans::ShaderStage::NumShaderStages;
	}
}

}