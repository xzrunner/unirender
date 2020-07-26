#pragma once

#include "unirender/DescriptorType.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace ur
{

class DescriptorPool : boost::noncopyable
{
public:
	virtual ~DescriptorPool() {}

}; // DescriptorPool

}