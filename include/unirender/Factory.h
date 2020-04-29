#pragma once

#include "unirender/RenderState.h"

#include <memory>

namespace ur
{

class Device;
class Context;

std::shared_ptr<Device> CreateDeviceGL();
std::shared_ptr<Context> CreateContextGL(const Device& device);

RenderState DefaultRenderState2D();

}