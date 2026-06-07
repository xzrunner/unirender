#include "unirender/Factory.h"

#include <exception>
#include <cstdlib>
#include <string>

#ifndef __APPLE__ // the OpenGL backend is excluded on macOS (Metal is used instead)
#include "unirender/opengl/Device.h"
#include "unirender/opengl/Context.h"
#endif
// Vulkan backend: built on all platforms. On macOS it runs via MoltenVK
// (Vulkan -> Metal); Metal stays the default and Vulkan is opt-in at runtime.
#include "unirender/vulkan/Device.h"
#include "unirender/vulkan/Context.h"
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
    {
#ifdef NDEBUG
        bool enable_validation_layers = false;
#else
        bool enable_validation_layers = true;
#endif
#ifdef __APPLE__
        // The Khronos validation layer is unstable on MoltenVK -- it can segfault
        // inside CmdPipelineBarrier validation during texture upload. Default it OFF
        // on macOS; opt in explicitly with TT_VK_VALIDATION=1.
        enable_validation_layers = false;
        if (const char* v = std::getenv("TT_VK_VALIDATION")) {
            enable_validation_layers = (std::string(v) == "1" || std::string(v) == "true");
        }
#endif
        try {
            ret = std::make_shared<vulkan::Device>(enable_validation_layers);
        } catch (const std::exception& e) {
            // Validation layers can enumerate yet fail to actually load
            // (stale Vulkan SDK registry entry, arch mismatch). Don't let that
            // hard-crash the app -- retry once with validation disabled.
            // A real driver/ICD problem (e.g. VK_ERROR_INCOMPATIBLE_DRIVER)
            // will throw again here and propagate, but with a decoded message.
            if (enable_validation_layers) {
                logger << "Vulkan: device creation with validation layers failed:\n  "
                       << e.what()
                       << "\nRetrying without validation layers..." << std::endl;
                ret = std::make_shared<vulkan::Device>(false);
            } else {
                throw;
            }
        }
    }
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
        ret = std::make_shared<vulkan::Context>(device, hwnd, width, height);
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