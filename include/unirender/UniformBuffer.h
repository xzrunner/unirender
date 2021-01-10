#pragma once

#include "unirender/noncopyable.h"

#include <vector>

namespace ur
{

class UniformBuffer : noncopyable
{
public:
	virtual ~UniformBuffer() {}

	virtual void Update(const void* data, size_t size) = 0;

}; // UniformBuffer

}