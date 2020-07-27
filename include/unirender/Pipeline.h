#pragma once

#include <boost/noncopyable.hpp>

namespace ur
{

class Pipeline : boost::noncopyable
{
public:
	virtual ~Pipeline() {}

}; // Pipeline

}