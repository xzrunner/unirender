#include "unirender/Factory.h"
#include "unirender/opengl/Device.h"
#include "unirender/opengl/Context.h"
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/Context.h"

namespace ur
{

std::shared_ptr<Device> CreateDevice(APIType type)
{
    std::shared_ptr<Device> ret = nullptr;
    switch (type)
    {
    case APIType::OpenGL:
        ret = std::make_shared<opengl::Device>();
        break;
    case APIType::Vulkan:
    {
#ifdef NDEBUG
        const bool enable_validation_layers = false;
#else
        const bool enable_validation_layers = true;
#endif
        ret = std::make_shared<vulkan::Device>(enable_validation_layers);
    }
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
        ret = std::make_shared<opengl::Context>(device);
        break;
    case APIType::Vulkan:
        ret = std::make_shared<vulkan::Context>(device, hwnd, width, height);
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