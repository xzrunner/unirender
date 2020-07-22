#pragma once

#include "unirender/Context.h"
#include "unirender/vulkan/VulkanContext.h"

#undef DrawState

namespace ur
{

class Device;

namespace vulkan
{

class Context : public ur::Context
{
public:
    Context(const ur::Device& device, void* hwnd, 
        uint32_t width, uint32_t height);
    virtual ~Context();

    virtual void Resize(uint32_t width, uint32_t height) override;

    virtual void Clear(const ClearState& clear_state) override;
    virtual void Draw(PrimitiveType prim_type, int offset, int count,
        const DrawState& draw, const void* scene) override;
    virtual void Draw(PrimitiveType prim_type, const DrawState& draw,
        const void* scene) override;

    virtual void SetViewport(int x, int y, int w, int h) override;
    virtual void GetViewport(int& x, int& y, int& w, int& h) const override;

    virtual void SetTexture(size_t slot, const ur::TexturePtr& tex) override;
    virtual void SetTextureSampler(size_t slot, const std::shared_ptr<ur::TextureSampler>& sampler) override;
	virtual void SetFramebuffer(const std::shared_ptr<ur::Framebuffer>& fb) override;
	virtual std::shared_ptr<ur::Framebuffer> GetFramebuffer() const override;

    virtual void SetUnpackRowLength(int len) override;
    virtual void SetPackRowLength(int len) override;

    virtual bool CheckRenderTargetStatus() override;

    virtual void Flush() override;

private:
	void Draw();

    void BuildCommandBuffers();

private:
    VulkanContext m_vk_ctx;

}; // Context

}
}