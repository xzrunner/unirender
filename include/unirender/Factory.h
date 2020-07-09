#pragma once

#include "unirender/RenderState.h"

#include <memory>

namespace ur
{

class Device;
class Context;

enum class APIType
{
	OpenGL,
	Vulkan,
};

std::shared_ptr<Device> CreateDevice(APIType type, void* hwnd);
std::shared_ptr<Context> CreateContext(APIType type, const Device& device);

RenderState DefaultRenderState2D();

}