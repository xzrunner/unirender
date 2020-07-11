#pragma once

#include "unirender/PrimitiveType.h"
#include "unirender/typedef.h"
#include "unirender/TextureTarget.h"

#include <memory>
#include <string>

namespace ur
{

struct ClearState;
struct DrawState;
struct RenderState;
struct Rectangle;
class Framebuffer;
class ShaderProgram;
class TextureUnit;
class TextureSampler;

class Context
{
public:
    virtual ~Context() {}

    virtual void Resize(uint32_t width, uint32_t height) = 0;

    virtual void Clear(const ClearState& clear_state) = 0;
    virtual void Draw(PrimitiveType prim_type, int offset, int count,
        const DrawState& draw, const void* scene) = 0;
    virtual void Draw(PrimitiveType prim_type, const DrawState& draw,
        const void* scene) = 0;

    virtual void SetViewport(int x, int y, int w, int h) = 0;
    virtual void GetViewport(int& x, int& y, int& w, int& h) const = 0;

    virtual void SetTexture(size_t slot, const ur::TexturePtr& tex) = 0;
    virtual void SetTextureSampler(size_t slot, const std::shared_ptr<ur::TextureSampler>& sampler) = 0;

    virtual void SetFramebuffer(const std::shared_ptr<Framebuffer>& fbo) = 0;
    virtual std::shared_ptr<Framebuffer> GetFramebuffer() const = 0;

    virtual void SetUnpackRowLength(int len) = 0;
    virtual void SetPackRowLength(int len) = 0;

    virtual bool CheckRenderTargetStatus() = 0;

}; // Context

}