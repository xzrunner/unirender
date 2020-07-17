#pragma once

#include "unirender/ShaderType.h"

#include <shadertrans/ShaderTrans.h>

namespace ur
{

class Adaptor
{
public:
	static shadertrans::ShaderStage ToShaderTransStage(ShaderType type);

}; // Adaptor

}