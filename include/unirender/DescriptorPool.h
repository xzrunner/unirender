#pragma once

#include <boost/noncopyable.hpp>

namespace ur
{

class DescriptorPool : boost::noncopyable
{
public:
	virtual ~DescriptorPool() {}

}; // DescriptorPool

}