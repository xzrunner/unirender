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

std::shared_ptr<Device> CreateDevice(APIType type);
std::shared_ptr<Context> CreateContext(APIType type, const Device& device, 
	void* hwnd = nullptr, uint32_t width = 0, uint32_t height = 0);

RenderState DefaultRenderState2D();

}