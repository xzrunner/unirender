#pragma once

#include "unirender/BufferUsageHint.h"
#include "unirender/TextureMinificationFilter.h"
#include "unirender/TextureMagnificationFilter.h"
#include "unirender/TextureWrap.h"
#include "unirender/TextureFormat.h"
#include "unirender/InternalFormat.h"
#include "unirender/AttachmentType.h"
#include "unirender/VertexLayoutType.h"
#include "unirender/typedef.h"
#include "unirender/DescriptorType.h"
#include "unirender/ShaderType.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace ur
{

class VertexArray;
class Framebuffer;
class RenderBuffer;
class ShaderProgram;
class VertexBuffer;
class IndexBuffer;
class Texture;
struct TextureDescription;
class TextureSampler;
class WritePixelBuffer;
class ComputeBuffer;
class DescriptorPool;
class DescriptorSetLayout;
class Pipeline;
class PipelineLayout;
class RenderPass;
class UniformBuffer;
class DescriptorSet;
struct Descriptor;

class Device
{
public:
    Device(std::ostream& logger = std::cerr);
    virtual ~Device() {}

    virtual int GetMaxNumVertAttrs() const = 0;
    virtual int GetMaxNumTexUnits() const = 0;
    virtual int GetMaxNumColorAttachments() const = 0;

    enum class PrimitiveType
    {
        Quad,
        Cube,
    };
    virtual std::shared_ptr<VertexArray>
        GetVertexArray(PrimitiveType prim, VertexLayoutType layout) const = 0;

    virtual std::shared_ptr<VertexArray> CreateVertexArray() const = 0;
    virtual std::shared_ptr<Framebuffer> CreateFramebuffer() const = 0;
    virtual std::shared_ptr<RenderBuffer> CreateRenderBuffer(
        int width, int height, InternalFormat format, AttachmentType attach) const = 0;

    virtual std::shared_ptr<ShaderProgram> CreateShaderProgram(
        const std::vector<unsigned int>& vs, 
        const std::vector<unsigned int>& fs,
        const std::vector<unsigned int>& tcs = std::vector<unsigned int>(),
        const std::vector<unsigned int>& tes = std::vector<unsigned int>()
    ) const = 0;
    virtual std::shared_ptr<ShaderProgram>
        CreateShaderProgram(const std::vector<unsigned int>& cs) const = 0;

    virtual std::shared_ptr<VertexBuffer>
        CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const = 0;
    virtual std::shared_ptr<IndexBuffer>
        CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const = 0;
    virtual std::shared_ptr<WritePixelBuffer>
        CreateWritePixelBuffer(BufferUsageHint hint, int size_in_bytes) const = 0;
    virtual std::shared_ptr<ComputeBuffer>
        CreateComputeBuffer(const std::vector<int>& buf, size_t index) const = 0;
    virtual std::shared_ptr<ComputeBuffer>
        CreateComputeBuffer(const std::vector<float>& buf, size_t index) const = 0;

    virtual std::shared_ptr<Texture>
        CreateTexture(const TextureDescription& desc, const void* pixels = nullptr) const = 0;
    virtual std::shared_ptr<Texture>
        CreateTexture(size_t width, size_t height, TextureFormat format, const void* buf, size_t buf_sz) const = 0;
	virtual std::shared_ptr<Texture>
		CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const = 0;
    virtual std::shared_ptr<TextureSampler>
        CreateTextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter, TextureWrap wrap_s, TextureWrap wrap_t, float max_anistropy = 1.0) const = 0;

    virtual std::shared_ptr<UniformBuffer>
        CreateUniformBuffer(const void* data, size_t size) const = 0;
    virtual std::shared_ptr<DescriptorPool>
        CreateDescriptorPool(size_t max_sets, const std::vector<std::pair<DescriptorType, size_t>>& pool_sizes) const = 0;
    virtual std::shared_ptr<DescriptorSetLayout>
        CreateDescriptorSetLayout(const std::vector<std::pair<DescriptorType, ShaderType>>& bindings) const = 0;
    virtual std::shared_ptr<DescriptorSet> CreateDescriptorSet(const DescriptorPool& pool, 
        const std::vector<std::shared_ptr<ur::DescriptorSetLayout>>& layouts, 
        const std::vector<ur::Descriptor>& descriptors) const = 0;
    virtual std::shared_ptr<VertexBuffer>
        CreateVertexBuffer(const void* data, size_t size) const = 0;

    virtual void DispatchCompute(int thread_group_count) const = 0;

    virtual void ReadPixels(const unsigned char* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const = 0;
    virtual void ReadPixels(const short* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const = 0;

    void Init();

    enum class TextureSamplerType
    {
        NearestClamp,
        LinearClamp,
        NearestRepeat,
        LinearRepeat,
    };
    std::shared_ptr<TextureSampler> 
        GetTextureSampler(TextureSamplerType type) const;

    void SetPipelineLayout(const std::string& name, const std::shared_ptr<PipelineLayout>& layout) {
        m_pipeline_layouts[name] = layout;
    }
    std::shared_ptr<PipelineLayout> GetPipelineLayout(const std::string& name) const {
        auto itr = m_pipeline_layouts.find(name);
        return itr == m_pipeline_layouts.end() ? nullptr : itr->second;
    }

    void SetRenderPass(const std::string& name, const std::shared_ptr<RenderPass>& pass) {
        m_render_passes[name] = pass;
    }
    std::shared_ptr<RenderPass> GetRenderPass(const std::string& name) const {
        auto itr = m_render_passes.find(name);
        return itr == m_render_passes.end() ? nullptr : itr->second;
    }

    void SetDescriptorSetLayout(const std::string& name, const std::shared_ptr<DescriptorSetLayout>& layout) {
        m_desc_set_layouts[name] = layout;
    }
    std::shared_ptr<DescriptorSetLayout> GetDescriptorSetLayout(const std::string& name) const {
        auto itr = m_desc_set_layouts.find(name);
        return itr == m_desc_set_layouts.end() ? nullptr : itr->second;
    }

    void SetPipeline(const std::string& name, const std::shared_ptr<Pipeline>& pipelines) {
        m_pipelines[name] = pipelines;
    }
    std::shared_ptr<Pipeline> GetPipeline(const std::string& name) const {
        auto itr = m_pipelines.find(name);
        return itr == m_pipelines.end() ? nullptr : itr->second;
    }

    void SetDescriptorPool(const std::shared_ptr<DescriptorPool>& desc_pool) { 
        m_desc_pool = desc_pool;
    }
    auto GetDescriptorPool() const { return m_desc_pool; }

    auto& GetLogger() const { return m_logger; }

private:
    std::unordered_map<std::string, std::shared_ptr<PipelineLayout>>      m_pipeline_layouts;
    std::unordered_map<std::string, std::shared_ptr<RenderPass>>          m_render_passes;
    std::unordered_map<std::string, std::shared_ptr<DescriptorSetLayout>> m_desc_set_layouts;
    std::unordered_map<std::string, std::shared_ptr<Pipeline>>            m_pipelines;

    std::shared_ptr<DescriptorPool> m_desc_pool = nullptr;

    std::shared_ptr<TextureSampler> m_nearest_clamp  = nullptr;
    std::shared_ptr<TextureSampler> m_linear_clamp   = nullptr;
    std::shared_ptr<TextureSampler> m_nearest_repeat = nullptr;
    std::shared_ptr<TextureSampler> m_linear_repeat  = nullptr;

    std::ostream& m_logger;

}; // Device

}