#include "unirender/Factory.h"
#include "unirender/opengl/Device.h"
#include "unirender/opengl/Context.h"
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/Context.h"

namespace ur
{

std::shared_ptr<Device> CreateDevice(APIType type, void* hwnd)
{
    std::shared_ptr<Device> ret = nullptr;
    switch (type)
    {
    case APIType::OpenGL:
        ret = std::make_shared<opengl::Device>();
        break;
    case APIType::Vulkan:
        ret = std::make_shared<vulkan::Device>(hwnd);
        break;
    }
    return ret;
}

std::shared_ptr<Context> CreateContext(APIType type, const Device& device)
{
    std::shared_ptr<Context> ret = nullptr;
    switch (type)
    {
    case APIType::OpenGL:
        ret = std::make_shared<opengl::Context>(device);
        break;
    case APIType::Vulkan:
        ret = std::make_shared<vulkan::Context>(device);
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