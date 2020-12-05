#include "unirender/opengl/Device.h"
#include "unirender/opengl/opengl.h"
#include "unirender/opengl/TextureSampler.h"
#include "unirender/opengl/ShaderProgram.h"
#include "unirender/opengl/VertexBuffer.h"
#include "unirender/opengl/IndexBuffer.h"
#include "unirender/opengl/Texture.h"
#include "unirender/opengl/WritePixelBuffer.h"
#include "unirender/opengl/VertexArray.h"
#include "unirender/opengl/Framebuffer.h"
#include "unirender/opengl/RenderBuffer.h"
#include "unirender/opengl/ComputeBuffer.h"
#include "unirender/opengl/TextureFormat.h"

#include <array>
#include <vector>

#include <assert.h>

namespace
{

struct vec2
{
    vec2() {}
    vec2(float x, float y) : x(x), y(y) {}

    vec2 operator - (const vec2& v) const {
        return vec2(x - v.x, y - v.y);
    }

    float x = 0, y = 0;

}; // vec2

struct vec3
{
    vec3() {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    vec3 operator - (const vec3& v) const {
        return vec3(x - v.x, y - v.y, z - v.z);
    }

    void Normalize() 
    {
        const float l = x * x + y * y + z * z;
        if (l > 0) 
        {
            const float s = 1.0f / sqrt(l);
            x *= s;
            y *= s;
            z *= s;
        }
    }

    float x = 0, y = 0, z = 0;

}; // vec3

}

namespace
{

void revert_y(uint8_t* pixels, int width, int height, int channel, int pixel_size)
{
    int line_sz = width * channel * pixel_size;
    std::vector<uint8_t> buf(line_sz, 0);
    int bpos = 0, epos = line_sz * (height - 1);
    for (int i = 0, n = (int)(floorf(height / 2.0f)); i < n; ++i) {
        memcpy(buf.data(), &pixels[bpos], line_sz);
        memcpy(&pixels[bpos], &pixels[epos], line_sz);
        memcpy(&pixels[epos], buf.data(), line_sz);
        bpos += line_sz;
        epos -= line_sz;
    }
}

}

namespace ur
{
namespace opengl
{

Device::Device(std::ostream& logger)
    : ur::Device(logger)
{
    Init();
}

std::shared_ptr<ur::VertexArray>
Device::GetVertexArray(PrimitiveType prim, VertexLayoutType layout) const
{
    const int layout_idx = static_cast<int>(layout);
    switch (prim)
    {
    case PrimitiveType::Quad:
    {
        if (m_quad_va[layout_idx]) {
            return m_quad_va[layout_idx];
        } else{
            auto va = CreateQuadVertexArray(layout);
            m_quad_va[layout_idx] = va;
            return va;
        }
    }
        break;
    case PrimitiveType::Cube:
    {
        if (m_cube_va[layout_idx]) {
            return m_cube_va[layout_idx];
        } else{
            auto va = CreateCubeVertexArray(layout);
            m_cube_va[layout_idx] = va;
            return va;
        }
    }
        break;
    }

    return nullptr;
}

std::shared_ptr<ur::VertexArray>
Device::CreateVertexArray() const
{
    return std::make_shared<ur::opengl::VertexArray>(*this);
}

std::shared_ptr<ur::Framebuffer>
Device::CreateFramebuffer() const
{
    return std::make_shared<ur::opengl::Framebuffer>(*this);
}

std::shared_ptr<ur::RenderBuffer>
Device::CreateRenderBuffer(int width, int height, InternalFormat format, AttachmentType attach) const
{
    return std::make_shared<ur::opengl::RenderBuffer>(width, height, format, attach);
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::vector<unsigned int>& vs, 
                            const std::vector<unsigned int>& fs,
                            const std::vector<unsigned int>& tcs,
                            const std::vector<unsigned int>& tes) const
{
    if (vs.empty() || fs.empty()) {
        return nullptr;
    } else {
        return std::make_shared<ur::opengl::ShaderProgram>(vs, fs, tcs, tes, GetLogger());
    }
}

std::shared_ptr<ur::ShaderProgram>
Device::CreateShaderProgram(const std::vector<unsigned int>& cs) const
{
    return std::make_shared<ur::opengl::ShaderProgram>(cs);
}

std::shared_ptr<ur::VertexBuffer>
Device::CreateVertexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
    return std::make_shared<ur::opengl::VertexBuffer>(usage_hint, size_in_bytes);
}

std::shared_ptr<ur::IndexBuffer>
Device::CreateIndexBuffer(BufferUsageHint usage_hint, int size_in_bytes) const
{
    return std::make_shared<ur::opengl::IndexBuffer>(usage_hint, size_in_bytes);
}

std::shared_ptr<ur::WritePixelBuffer>
Device::CreateWritePixelBuffer(BufferUsageHint hint, int size_in_bytes) const
{
    return std::make_shared<ur::opengl::WritePixelBuffer>(hint, size_in_bytes);
}

std::shared_ptr<ur::ComputeBuffer>
Device::CreateComputeBuffer(const std::vector<int>& buf, size_t index) const
{
    return std::make_shared<ur::opengl::ComputeBuffer>(buf, index);
}

std::shared_ptr<ur::ComputeBuffer>
Device::CreateComputeBuffer(const std::vector<float>& buf, size_t index) const
{
    return std::make_shared<ur::opengl::ComputeBuffer>(buf, index);
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(const TextureDescription& desc, const void* pixels) const
{
    auto tex = std::make_shared<ur::opengl::Texture>(desc, *this);
    if (pixels) {
        tex->ReadFromMemory(pixels, desc.format, desc.width, desc.height, desc.depth, 1);
    }
    return tex;
}

std::shared_ptr<ur::Texture>
Device::CreateTexture(size_t width, size_t height, ur::TextureFormat format, const void* buf, size_t buf_sz) const
{
    auto pbuf = CreateWritePixelBuffer(ur::BufferUsageHint::StreamDraw, buf_sz);
    pbuf->ReadFromMemory(buf, buf_sz, 0);

    TextureDescription desc;
    desc.target = ur::TextureTarget::Texture2D;
    desc.width  = width;
    desc.height = height;
    desc.format = format;

    auto tex = std::make_shared<ur::opengl::Texture>(desc, *this);
    tex->ReadFromMemory(*pbuf, 0, 0, width, height, 1);

    return tex;
}

std::shared_ptr<ur::Texture>
Device::CreateTextureCubeMap(const std::array<TexturePtr, 6>& textures) const
{
    ur::TextureDescription desc;
    desc.target = ur::TextureTarget::TextureCubeMap;
    desc.format = ur::TextureFormat::RGB;

    WritePixelBuffer::UnBind();

    auto tex = std::make_shared<ur::opengl::Texture>(desc, *this);
    for (int i = 0; i < 6; ++i)
    {
        auto& src = textures[i];
        if (!src) {
            continue;
        }

        auto w = src->GetWidth();
        auto h = src->GetHeight();

        //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        //glCopyImageSubData(src->GetTexID(), GL_TEXTURE_2D, 0, 0, 0, 0, tex->GetTexID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0, src->GetWidth(), src->GetHeight(), 0);

        auto fmt = src->GetFormat();
		switch (fmt)
		{
		case ur::TextureFormat::RGB:
		{
			GLubyte* pixels = new GLubyte[w * h * 3];
			glBindTexture(GL_TEXTURE_2D, src->GetTexID());
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			revert_y(pixels, w, h, 3, 1);

			glBindTexture(GL_TEXTURE_CUBE_MAP, tex->GetTexID());
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			delete[] pixels;
		}
			break;
		case ur::TextureFormat::RGB16F:
		{
			GLfloat* pixels = new GLfloat[w * h * 3];
			glBindTexture(GL_TEXTURE_2D, src->GetTexID());
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, pixels);
			revert_y((GLubyte*)(pixels), w, h, 3, 4);

			glBindTexture(GL_TEXTURE_CUBE_MAP, tex->GetTexID());
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, pixels);
			delete[] pixels;
		}
			break;
		default:
			assert(0);
		}
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tex;
}

std::shared_ptr<ur::TextureSampler>
Device::CreateTextureSampler(TextureMinificationFilter min_filter, TextureMagnificationFilter mag_filter, 
                             TextureWrap wrap_s, TextureWrap wrap_t, float max_anistropy) const
{
    return std::make_shared<ur::opengl::TextureSampler>(min_filter, mag_filter, wrap_s, wrap_t, max_anistropy);
}

void Device::DispatchCompute(int num_groups_x, int num_groups_y, int num_groups_z) const
{
    glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Device::ReadPixels(const unsigned char* pixels, ur::TextureFormat format,
                        int x, int y, int w, int h) const
{
    TextureFormat gl_fmt(format);
    glReadPixels(x, y, w, h, gl_fmt.internal_format, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
}

void Device::ReadPixels(const short* pixels, ur::TextureFormat format,
                        int x, int y, int w, int h) const
{
    TextureFormat gl_fmt(format);
    glReadPixels(x, y, w, h, gl_fmt.internal_format, GL_SHORT, (GLvoid*)pixels);
}

void Device::Init()
{
#if OPENGLES < 2
    glewInit();
#endif

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_max_num_vert_attrs);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_max_num_tex_units);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &m_max_num_color_attachments);
}

std::shared_ptr<ur::VertexArray>
Device::CreateQuadVertexArray(VertexLayoutType layout) const
{
    const bool unit = false;
    const auto p_min = unit ? 0.0f : -1.0f;

    std::vector<float> vertices;
    switch (layout)
    {
    case VertexLayoutType::Pos:
        vertices = {
            // positions
            p_min, 1.0f,  0.0f,
            p_min, p_min, 0.0f,
            1.0f,  1.0f,  0.0f,
            1.0f,  p_min, 0.0f,
        };
        break;
    case VertexLayoutType::PosTex:
        vertices = {
            // positions        // texture Coords
            p_min, 1.0f,  0.0f, 0.0f, 1.0f,
            p_min, p_min, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
            1.0f,  p_min, 0.0f, 1.0f, 0.0f,
        };
        break;
    case VertexLayoutType::PosNorm:
        vertices = {
            // positions        // normal
            p_min, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f,
            p_min, p_min, 0.0f, 0.0f, 0.0f, 1.0f,
            1.0f,  1.0f,  0.0f, 0.0f, 0.0f, 1.0f,
            1.0f,  p_min, 0.0f, 0.0f, 0.0f, 1.0f,
        };
        break;
    case VertexLayoutType::PosNormTex:
        vertices = {
            // positions        // normal         // texture Coords
            p_min, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            p_min, p_min, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            1.0f,  p_min, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        };
        break;
    case VertexLayoutType::PosNormTexTB:
    {
        // positions
        vec3 pos1(p_min,  1.0f, 0.0f);
        vec3 pos2(p_min, p_min, 0.0f);
        vec3 pos3( 1.0f, p_min, 0.0f);
        vec3 pos4( 1.0f,  1.0f, 0.0f);
        // texture coordinates
        vec2 uv1(0.0f, 1.0f);
        vec2 uv2(0.0f, 0.0f);
        vec2 uv3(1.0f, 0.0f);
        vec2 uv4(1.0f, 1.0f);
        // normal vector
        vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        vec3 tangent1, bitangent1;
        vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        vec3 edge1 = pos2 - pos1;
        vec3 edge2 = pos3 - pos1;
        vec2 deltaUV1 = uv2 - uv1;
        vec2 deltaUV2 = uv3 - uv1;

        GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1.Normalize();

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1.Normalize();

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2.Normalize();

        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2.Normalize();

        vertices = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
    }
        break;
    default:
        assert(0);
    }

    auto va = CreateVertexArray();

    auto usage = ur::BufferUsageHint::StaticDraw;

    auto vbuf_sz = sizeof(float) * vertices.size();
    auto vbuf = CreateVertexBuffer(ur::BufferUsageHint::StaticDraw, vbuf_sz);
    vbuf->ReadFromMemory(vertices.data(), vbuf_sz, 0);
    va->SetVertexBuffer(vbuf);

    std::vector<std::shared_ptr<ur::VertexInputAttribute>> vbuf_attrs;
    switch (layout)
    {
    case VertexLayoutType::Pos:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 12
        ));
        break;
    case VertexLayoutType::PosTex:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 20
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 2, 12, 20
        ));
        break;
    case VertexLayoutType::PosNorm:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 24
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 24
        ));
        break;
    case VertexLayoutType::PosNormTex:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 32
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 32
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            2, ur::ComponentDataType::Float, 2, 24, 32
        ));
        break;
    case VertexLayoutType::PosNormTexTB:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 56
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 56
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            2, ur::ComponentDataType::Float, 2, 24, 56
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            3, ur::ComponentDataType::Float, 3, 32, 56
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            4, ur::ComponentDataType::Float, 3, 44, 56
        ));
        break;
    }
    va->SetVertexBufferAttrs(vbuf_attrs);

    return va;
}

std::shared_ptr<ur::VertexArray>
Device::CreateCubeVertexArray(VertexLayoutType layout) const
{
    std::vector<float> vertices;
    switch (layout)
    {
    case VertexLayoutType::Pos:
        vertices = {
            // back face
            -1.0f, -1.0f, -1.0f, // bottom-left
             1.0f,  1.0f, -1.0f, // top-right
             1.0f, -1.0f, -1.0f, // bottom-right
             1.0f,  1.0f, -1.0f, // top-right
            -1.0f, -1.0f, -1.0f, // bottom-left
            -1.0f,  1.0f, -1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f, // bottom-left
             1.0f, -1.0f,  1.0f, // bottom-right
             1.0f,  1.0f,  1.0f, // top-right
             1.0f,  1.0f,  1.0f, // top-right
            -1.0f,  1.0f,  1.0f, // top-left
            -1.0f, -1.0f,  1.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, // top-right
            -1.0f,  1.0f, -1.0f, // top-left
            -1.0f, -1.0f, -1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f, // top-left
             1.0f, -1.0f, -1.0f, // bottom-right
             1.0f,  1.0f, -1.0f, // top-right
             1.0f, -1.0f, -1.0f, // bottom-right
             1.0f,  1.0f,  1.0f, // top-left
             1.0f, -1.0f,  1.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, // top-right
             1.0f, -1.0f, -1.0f, // top-left
             1.0f, -1.0f,  1.0f, // bottom-left
             1.0f, -1.0f,  1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, // bottom-right
            -1.0f, -1.0f, -1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f, // top-left
             1.0f,  1.0f , 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f, // top-right
             1.0f,  1.0f,  1.0f, // bottom-right
            -1.0f,  1.0f, -1.0f, // top-left
            -1.0f,  1.0f,  1.0f  // bottom-left
        };
        break;
    case VertexLayoutType::PosTex:
        vertices = {
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f, 0.0f, 0.0f  // bottom-left
        };
        break;
    case VertexLayoutType::PosNorm:
        vertices = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f  // bottom-left
        };
        break;
    case VertexLayoutType::PosNormTex:
        vertices = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        break;
    default:
        assert(0);
    }

    auto va = CreateVertexArray();

    auto usage = ur::BufferUsageHint::StaticDraw;

    auto vbuf_sz = sizeof(float) * vertices.size();
    auto vbuf = CreateVertexBuffer(ur::BufferUsageHint::StaticDraw, vbuf_sz);
    vbuf->ReadFromMemory(vertices.data(), vbuf_sz, 0);
    va->SetVertexBuffer(vbuf);

    std::vector<std::shared_ptr<ur::VertexInputAttribute>> vbuf_attrs;
    switch (layout)
    {
    case VertexLayoutType::Pos:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 12
        ));
        break;
    case VertexLayoutType::PosTex:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 20
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 2, 12, 20
        ));
        break;
    case VertexLayoutType::PosNorm:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 24
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 24
        ));
        break;
    case VertexLayoutType::PosNormTex:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 32
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 32
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            2, ur::ComponentDataType::Float, 2, 24, 32
        ));
        break;
    case VertexLayoutType::PosNormTexTB:
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 56
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 56
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            2, ur::ComponentDataType::Float, 2, 24, 56
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            3, ur::ComponentDataType::Float, 3, 32, 56
        ));
        vbuf_attrs.push_back(std::make_shared<ur::VertexInputAttribute>(
            4, ur::ComponentDataType::Float, 3, 44, 56
        ));
        break;
    }
    va->SetVertexBufferAttrs(vbuf_attrs);

    return va;
}

}
}