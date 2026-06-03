#include "unirender/Factory.h"
#ifndef __APPLE__ // the OpenGL backend is excluded on macOS (Metal is used instead)
#include "unirender/opengl/Device.h"
#include "unirender/opengl/Context.h"
#endif
#ifndef __APPLE__ // the Vulkan backend is excluded on macOS (Metal is used instead)
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/Context.h"
#endif
#ifdef __APPLE__
#include "unirender/metal/Device.h"
#include "unirender/metal/Context.h"
#endif

namespace ur
{

std::shared_ptr<Device> CreateDevice(APIType type, std::ostream& logger)
{
    std::shared_ptr<Device> ret = nullptr;
    switch (type)
    {
    case APIType::OpenGL:
#ifndef __APPLE__
        ret = std::make_shared<opengl::Device>(logger);
#endif
        break;
    case APIType::Vulkan:
#ifndef __APPLE__
    {
#ifdef NDEBUG
        const bool enable_validation_layers = false;
#else
        const bool enable_validation_layers = true;
#endif
        ret = std::make_shared<vulkan::Device>(enable_validation_layers);
    }
#endif
        break;
    case APIType::Metal:
#ifdef __APPLE__
        ret = std::make_shared<metal::Device>(logger);
#endif
        break;
    }
    return ret;
}

std::shared_ptr<Context> CreateContext(APIType type, const Device& device, void* hwnd, 
                                       uint32_t width, uint32_t height)
{
    std::shared_ptr<Context> ret = nullptr;
    switch (type)
    {
    case APIType::OpenGL:
#ifndef __APPLE__
        ret = std::make_shared<opengl::Context>(device);
#endif
        break;
    case APIType::Vulkan:
#ifndef __APPLE__
        ret = std::make_shared<vulkan::Context>(device, hwnd, width, height);
#endif
        break;
    case APIType::Metal:
#ifdef __APPLE__
        ret = std::make_shared<metal::Context>(device, hwnd, width, height);
#endif
        break;
    }
    return ret;
}

RenderState DefaultRenderState2D()
{
    ur::RenderState rs;

    rs.depth_test.enabled    = false;
    rs.facet_culling.enabled = false;

    rs.blending.enabled    = true;
    rs.blending.separately = false;
    rs.blending.src        = ur::BlendingFactor::SrcAlpha;
    rs.blending.dst        = ur::BlendingFactor::OneMinusSrcAlpha;
    rs.blending.equation   = ur::BlendEquation::Add;

    return rs;
}

}