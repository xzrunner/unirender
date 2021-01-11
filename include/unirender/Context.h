#pragma once

#include "unirender/PrimitiveType.h"
#include "unirender/typedef.h"
#include "unirender/TextureTarget.h"
#include "unirender/BarrierType.h"
#include "unirender/AccessType.h"
#include "unirender/noncopyable.h"

#include <memory>
#include <string>
#include <vector>

#ifdef DrawState
#undef DrawState
#endif

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
class DescriptorPool;
class Pipeline;
class PipelineLayout;
class VertexBuffer;

class Context : noncopyable
{
public:
    virtual ~Context() {}

    virtual void Resize(uint32_t width, uint32_t height) = 0;

    virtual void Clear(const ClearState& clear_state) = 0;
    virtual void Draw(PrimitiveType prim_type, int offset, int count,
        const DrawState& draw, const void* scene) = 0;
    virtual void Draw(PrimitiveType prim_type, const DrawState& draw,
        const void* scene) = 0;
    virtual void Compute(const DrawState& draw, int num_groups_x, 
        int num_groups_y, int num_groups_z) = 0;

    virtual void SetViewport(int x, int y, int w, int h) = 0;
    virtual void GetViewport(int& x, int& y, int& w, int& h) const = 0;

    virtual void SetTexture(size_t slot, const ur::TexturePtr& tex) = 0;
    virtual void SetTextureSampler(size_t slot, const std::shared_ptr<ur::TextureSampler>& sampler) = 0;
    virtual void SetImage(size_t slot, const ur::TexturePtr& tex, AccessType access) = 0;

    virtual void SetFramebuffer(const std::shared_ptr<Framebuffer>& fbo) = 0;
    virtual std::shared_ptr<Framebuffer> GetFramebuffer() const = 0;

    virtual void SetUnpackRowLength(int len) = 0;
    virtual void SetPackRowLength(int len) = 0;

    virtual bool CheckRenderTargetStatus() = 0;

    virtual void Flush() = 0;

    virtual std::shared_ptr<Pipeline> CreatePipeline(bool include_depth, bool include_vi, const ur::PipelineLayout& layout,
        const ur::VertexBuffer& vb, const ur::ShaderProgram& prog) const = 0;

    virtual void SetMemoryBarrier(const std::vector<BarrierType>& types) = 0;

}; // Context

}