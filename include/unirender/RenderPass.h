#pragma once

#include <boost/noncopyable.hpp>

namespace ur
{

class RenderPass : boost::noncopyable
{
public:
	virtual ~RenderPass() {}

}; // RenderPass

}