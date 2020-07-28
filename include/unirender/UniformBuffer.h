#pragma once

#include <boost/noncopyable.hpp>

#include <vector>

namespace ur
{

class UniformBuffer : boost::noncopyable
{
public:
	virtual ~UniformBuffer() {}

	virtual void Update(const void* data, size_t size) = 0;

}; // UniformBuffer

}