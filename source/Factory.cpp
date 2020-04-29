#include "unirender/Factory.h"
#include "unirender/opengl/Device.h"
#include "unirender/opengl/Context.h"

namespace ur
{

std::shared_ptr<Device> CreateDeviceGL()
{
    return std::make_shared<opengl::Device>();
}

std::shared_ptr<Context> CreateContextGL(const Device& device)
{
    return std::make_shared<opengl::Context>(device);
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