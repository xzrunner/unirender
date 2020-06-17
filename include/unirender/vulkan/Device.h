#pragma once

#include "unirender/Device.h"

#ifndef NOMINMAX
#define NOMINMAX /* Don't let Windows define min() or max() */
#endif

#include <vulkan/vulkan.h>

namespace ur
{
namespace vulkan
{

class Device : public ur::Device
{
public:
    Device();

    virtual int GetMaxNumVertAttrs() const override { return m_max_num_vert_attrs; }
    virtual int GetMaxNumTexUnits() const override { return m_max_num_tex_units; }
    virtual int GetMaxNumColorAttachments() const override { return m_max_num_color_attachments; }

    virtual std::shared_ptr<VertexArray>
        GetVertexArray(PrimitiveType prim, VertexLayoutType layout) const override;

    virtual std::shared_ptr<VertexArray> CreateVertexArray() const override;
    virtual std::shared_ptr<Framebuffer> CreateFramebuffer() const override;
    virtual std::shared_ptr<RenderBuffer> CreateRenderBuffer(
        int width, int height, InternalFormat format, AttachmentType attach) const override;

    virtual std::shared_ptr<ShaderProgram> CreateShaderProgram(
        const std::string& vs, const std::string& fs, const std::string& gs = "",
        const std::vector<std::string>& attr_names = std::vector<std::string>()) const override;
    virtual std::shared_ptr<ShaderProgram>
        CreateShaderProgram(const std::string& cs) const override;

    virtual std::shared_ptr<VertexBuffer>
        CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const override;
    virtual std::shared_ptr<IndexBuffer>
        CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const override;
    virtual std::shared_ptr<WritePixelBuffer>
        CreateWritePixelBuffer(BufferUsageHint hint, int size_in_bytes) const override;
    virtual std::shared_ptr<ComputeBuffer>
        CreateComputeBuffer(const std::vector<int>& buf, size_t index) const override;
    virtual std::shared_ptr<ComputeBuffer>
        CreateComputeBuffer(const std::vector<float>& buf, size_t index) const override;

    virtual std::shared_ptr<ur::Texture>
        CreateTexture(const TextureDescription& desc, const void* pixels = nullptr) const override;
    virtual std::shared_ptr<ur::Texture>
        CreateTexture(const Bitmap& bmp, TextureFormat format) const override;
    virtual std::shared_ptr<TextureSampler>
        CreateTextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter, TextureWrap wrap_s, TextureWrap wrap_t) const override;
    virtual std::shared_ptr<TextureSampler>
        GetTextureSampler(TextureSamplerType type) const override;

    virtual void DispatchCompute(int thread_group_count) const override;

    virtual void ReadPixels(const unsigned char* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const override;
    virtual void ReadPixels(const short* pixels, ur::TextureFormat fmt,
        int x, int y, int w, int h) const override;

private:
    /*
     * A layer can expose extensions, keep track of those
     * extensions here.
     */
    typedef struct {
        VkLayerProperties properties;
        std::vector<VkExtensionProperties> instance_extensions;
        std::vector<VkExtensionProperties> device_extensions;
    } layer_properties;

private:
    void Init();

    VkResult InitGlobalLayerProperties();
    void InitInstanceExtensionNames();
    void InitDeviceExtensionNames();
    VkResult InitInstance(const char* title);
    VkResult InitEnumerateDevice(uint32_t gpu_count = 1);
    void InitSwapchainExtension();
    VkResult InitDevice();

    VkResult InitGlobalExtensionProperties(layer_properties& layer_props);
    VkResult InitDeviceExtensionProperties(layer_properties& layer_props);

private:
    int m_max_num_vert_attrs = 0;
    int m_max_num_tex_units = 0;
    int m_max_num_color_attachments = 0;

    std::vector<const char*> m_instance_layer_names;
    std::vector<const char*> m_instance_extension_names;
    std::vector<layer_properties> m_instance_layer_properties;
    std::vector<VkExtensionProperties> m_instance_extension_properties;
    VkInstance m_inst;

    std::vector<const char*> m_device_extension_names;
    std::vector<VkExtensionProperties> m_device_extension_properties;
    std::vector<VkPhysicalDevice> m_gpus;
    VkDevice m_device;
    uint32_t m_graphics_queue_family_index;
    uint32_t m_present_queue_family_index;
    VkPhysicalDeviceProperties m_gpu_props;
    std::vector<VkQueueFamilyProperties> m_queue_props;
    VkPhysicalDeviceMemoryProperties m_memory_properties;

    int m_width, m_height;
    VkFormat m_format;

    uint32_t m_queue_family_count;

}; // Device
}
}