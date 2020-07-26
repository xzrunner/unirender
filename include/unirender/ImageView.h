#pragma once

#include <boost/noncopyable.hpp>

namespace ur
{

class ImageView : boost::noncopyable
{
public:
	virtual ~ImageView() {}

}; // ImageView

}