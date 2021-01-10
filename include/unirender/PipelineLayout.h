#pragma once

#include "unirender/noncopyable.h"

namespace ur
{
class PipelineLayout : noncopyable
{
public:
	virtual ~PipelineLayout() {}
}; // PipelineLayout
}