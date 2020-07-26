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

class Device
{
public:
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

    virtual std::shared_ptr<ShaderProgram> 
        CreateShaderProgram(const std::vector<unsigned int>& vs, const std::vector<unsigned int>& fs) const = 0;
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
        CreateTextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter, TextureWrap wrap_s, TextureWrap wrap_t) const = 0;

    virtual std::shared_ptr<DescriptorPool>
        CreateDescriptorPool(size_t max_sets, const std::vector<std::pair<DescriptorType, size_t>>& pool_sizes) const = 0;
    virtual std::shared_ptr<DescriptorSetLayout>
        CreateDescriptorSetLayout(const std::vector<std::pair<DescriptorType, ShaderType>>& bindings) const = 0;

    enum class TextureSamplerType
    {
        NearestClamp,
        LinearClamp,
        NearestRepeat,
        LinearRepeat,
    };
    virtual std::shared_ptr<TextureSampler>
        GetTextureSampler(TextureSamplerType type) const = 0;

    virtual void DispatchCompute(int thread_group_count) const = 0;

    virtual void ReadPixels(const unsigned char* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const = 0;
    virtual void ReadPixels(const short* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const = 0;

    void SetDescriptorSetLayout(const std::string& name, const std::shared_ptr<DescriptorSetLayout>& layout) {
        m_desc_set_layouts[name] = layout;
    }
    std::shared_ptr<DescriptorSetLayout> GetDescriptorSetLayout(const std::string& name) const {
        auto itr = m_desc_set_layouts.find(name);
        return itr == m_desc_set_layouts.end() ? nullptr : itr->second;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<DescriptorSetLayout>> m_desc_set_layouts;

}; // Device

}