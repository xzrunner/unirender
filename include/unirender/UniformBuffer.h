#pragma once

#include <boost/noncopyable.hpp>

#include <vector>

namespace ur
{

class UniformBuffer : boost::noncopyable
{
public:
	virtual ~UniformBuffer() {}

}; // UniformBuffer

}