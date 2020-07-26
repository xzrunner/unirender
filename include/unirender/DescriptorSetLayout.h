#pragma once

#include <boost/noncopyable.hpp>

namespace ur
{

class DescriptorSetLayout : boost::noncopyable
{
public:
	virtual ~DescriptorSetLayout() {}

}; // DescriptorSetLayout

}