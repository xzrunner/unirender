#pragma once

#include "unirender/ClearState.h"
#include "unirender/BufferTarget.h"
#include "unirender/BufferUsageHint.h"
#include "unirender/FacetCulling.h"
#include "unirender/RenderState.h"
#include "unirender/ComponentDataType.h"
#include "unirender/ShaderType.h"
#include "unirender/TextureMinificationFilter.h"
#include "unirender/TextureMagnificationFilter.h"
#include "unirender/TextureWrap.h"
#include "unirender/TextureTarget.h"
#include "unirender/AttachmentType.h"
#include "unirender/InternalFormat.h"
#include "unirender/PrimitiveType.h"
#include "unirender/IndexBuffer.h"
#include "unirender/AccessType.h"
#include "unirender/BarrierType.h"
#include "unirender/opengl/opengl.h"

namespace ur
{
namespace opengl
{

class TypeConverter
{
public:
    static GLbitfield To(const ClearBuffers& mask)
    {
        GLbitfield ret = 0;
        if (static_cast<int>(mask) & static_cast<int>(ClearBuffers::ColorBuffer)) {
            ret |= GL_COLOR_BUFFER_BIT;
        }
        if (static_cast<int>(mask) & static_cast<int>(ClearBuffers::DepthBuffer)) {
            ret |= GL_DEPTH_BUFFER_BIT;
        }
        if (static_cast<int>(mask) & static_cast<int>(ClearBuffers::StencilBuffer)) {
            ret |= GL_STENCIL_BUFFER_BIT;
        }
        return ret;
    }

    static GLenum To(CullFace face)
    {
        const GLenum faces[] = {
            GL_FRONT,
            GL_BACK,
            GL_FRONT_AND_BACK
        };
        return faces[static_cast<int>(face)];
    }

    static GLenum To(WindingOrder order)
    {
        const GLenum orders[] = {
            GL_CW,
            GL_CCW
        };
        return orders[static_cast<int>(order)];
    }

    static GLenum To(RasterizationMode mode)
    {
        const GLenum modes[] = {
            GL_POINT,
            GL_LINE,
            GL_FILL,
        };
        return modes[static_cast<int>(mode)];
    }

    static GLenum To(StencilOperation stencil)
    {
        const GLenum ops[] = {
            GL_ZERO,
            GL_INVERT,
            GL_KEEP,
            GL_REPLACE,
            GL_INCR,
            GL_DECR,
            GL_INCR_WRAP,
            GL_DECR_WRAP,
        };
        return ops[static_cast<int>(stencil)];
    }

    static GLenum To(StencilTestFunction func)
    {
        const GLenum funcs[] = {
            GL_NEVER,
            GL_LESS,
            GL_EQUAL,
            GL_LEQUAL,
            GL_GREATER,
            GL_NOTEQUAL,
            GL_GEQUAL,
            GL_ALWAYS,
        };
        return funcs[static_cast<int>(func)];
    }

    static GLenum To(DepthTestFunc func)
    {
        const GLenum funcs[] = {
            GL_NEVER,
            GL_LESS,
            GL_EQUAL,
            GL_LEQUAL,
            GL_GREATER,
            GL_NOTEQUAL,
            GL_GEQUAL,
            GL_ALWAYS,
        };
        return funcs[static_cast<int>(func)];
    }

    static GLenum To(BlendingFactor factor)
    {
        const GLenum factors[] = {
            GL_ZERO,
            GL_ONE,
            GL_SRC_COLOR,
            GL_ONE_MINUS_SRC_COLOR,
            GL_DST_COLOR,
            GL_ONE_MINUS_DST_COLOR,
            GL_SRC_ALPHA,
            GL_ONE_MINUS_SRC_ALPHA,
            GL_DST_ALPHA,
            GL_ONE_MINUS_DST_ALPHA,
            GL_CONSTANT_COLOR,
            GL_ONE_MINUS_CONSTANT_COLOR,
            GL_CONSTANT_ALPHA,
            GL_ONE_MINUS_CONSTANT_ALPHA,
            GL_SRC_ALPHA_SATURATE,
            GL_SRC1_COLOR,
            GL_ONE_MINUS_SRC1_COLOR,
            GL_SRC1_ALPHA,
            GL_ONE_MINUS_SRC1_ALPHA,
        };
        return factors[static_cast<int>(factor)];
    }

    static GLenum To(BlendEquation func)
    {
        const GLenum funcs[] = {
            GL_FUNC_ADD,
            GL_MIN,
            GL_MAX,
            GL_FUNC_SUBTRACT,
            GL_FUNC_REVERSE_SUBTRACT,
        };
        return funcs[static_cast<int>(func)];
    }

    static GLenum To(BufferTarget target)
    {
        const GLenum targets[] = {
            GL_ARRAY_BUFFER,
            GL_ELEMENT_ARRAY_BUFFER,
            GL_PIXEL_PACK_BUFFER,
            GL_PIXEL_UNPACK_BUFFER,
            GL_UNIFORM_BUFFER,
            GL_TEXTURE_BUFFER,
            GL_TRANSFORM_FEEDBACK_BUFFER,
            GL_COPY_READ_BUFFER,
            GL_COPY_WRITE_BUFFER,
            GL_SHADER_STORAGE_BUFFER,
        };
        return targets[static_cast<int>(target)];
    }

    static GLenum To(BufferUsageHint hint)
    {
        const GLenum hints[] = {
            GL_STREAM_DRAW,
            GL_STREAM_READ,
            GL_STREAM_COPY,
            GL_STATIC_DRAW,
            GL_STATIC_READ,
            GL_STATIC_COPY,
            GL_DYNAMIC_DRAW,
            GL_DYNAMIC_READ,
            GL_DYNAMIC_COPY,
        };
        return hints[static_cast<int>(hint)];
    }

    static GLenum To(ComponentDataType type)
    {
        const GLenum types[] = {
            GL_BYTE,
            GL_UNSIGNED_BYTE,
            GL_SHORT,
            GL_UNSIGNED_SHORT,
            GL_INT,
            GL_UNSIGNED_INT,
            GL_FLOAT,
            GL_HALF_FLOAT,
        };
        return types[static_cast<int>(type)];
    }

    static GLenum To(ShaderType type)
    {
        const GLenum types[] = {
            GL_VERTEX_SHADER,
            GL_TESS_CONTROL_SHADER,
            GL_TESS_EVALUATION_SHADER,
            GL_GEOMETRY_SHADER,
            GL_FRAGMENT_SHADER,
            GL_COMPUTE_SHADER,
        };
        return types[static_cast<int>(type)];
    }

    static GLenum To(TextureMinificationFilter filter)
    {
        const GLenum filters[] = {
            GL_NEAREST,
            GL_LINEAR,
            GL_NEAREST_MIPMAP_NEAREST,
            GL_LINEAR_MIPMAP_NEAREST,
            GL_NEAREST_MIPMAP_LINEAR,
            GL_LINEAR_MIPMAP_LINEAR,
        };
        return filters[static_cast<int>(filter)];
    }

    static GLenum To(TextureMagnificationFilter filter)
    {
        const GLenum filters[] = {
            GL_NEAREST,
            GL_LINEAR,
        };
        return filters[static_cast<int>(filter)];
    }

    static GLenum To(TextureWrap wrap)
    {
        const GLenum wraps[] = {
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_BORDER,
            GL_REPEAT,
            GL_MIRRORED_REPEAT,
        };
        return wraps[static_cast<int>(wrap)];
    }

    static GLenum To(TextureTarget target)
    {
        const GLenum targets[] = {
            GL_TEXTURE_1D,
            GL_TEXTURE_2D,
            GL_TEXTURE_3D,
            GL_TEXTURE_CUBE_MAP,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB,
        };
        return targets[static_cast<int>(target)];
    }

    static GLenum To(AlphaTestFunc func)
    {
        const GLenum funcs[] = {
            GL_NEVER,
            GL_LESS,
            GL_EQUAL,
            GL_LEQUAL,
            GL_GREATER,
            GL_NOTEQUAL,
            GL_GEQUAL,
            GL_ALWAYS,
        };
        return funcs[static_cast<int>(func)];
    }

    static GLenum To(AttachmentType type)
    {
        const GLenum types[] = {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
            GL_COLOR_ATTACHMENT4,
            GL_COLOR_ATTACHMENT5,
            GL_COLOR_ATTACHMENT6,
            GL_COLOR_ATTACHMENT7,
            GL_COLOR_ATTACHMENT8,
            GL_COLOR_ATTACHMENT9,
            GL_COLOR_ATTACHMENT10,
            GL_COLOR_ATTACHMENT11,
            GL_COLOR_ATTACHMENT12,
            GL_COLOR_ATTACHMENT13,
            GL_COLOR_ATTACHMENT14,
            GL_COLOR_ATTACHMENT15,
            GL_DEPTH_ATTACHMENT,
            GL_STENCIL_ATTACHMENT,
        };
        return types[static_cast<int>(type)];
    }

    static GLenum To(InternalFormat format)
    {
        const GLenum formats[] = {
            GL_DEPTH_COMPONENT,
            GL_DEPTH_STENCIL,
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
        };
        return formats[static_cast<int>(format)];
    }

    static GLenum To(PrimitiveType type)
    {
        const GLenum types[] = {
            GL_POINTS,
            GL_LINES,
            GL_LINE_LOOP,
            GL_LINE_STRIP,
            GL_TRIANGLES,
            GL_TRIANGLE_STRIP,
            GL_TRIANGLE_FAN,
            GL_LINES_ADJACENCY,
            GL_LINE_STRIP_ADJACENCY,
            GL_TRIANGLES_ADJACENCY,
            GL_TRIANGLE_STRIP_ADJACENCY,
            GL_PATCHES
        };
        return types[static_cast<int>(type)];
    }

    static GLenum To(IndexBufferDataType type)
    {
        const GLenum types[] = {
            GL_UNSIGNED_SHORT,
            GL_UNSIGNED_INT,
        };
        return types[static_cast<int>(type)];
    }

    static GLenum To(AccessType type)
    {
        const GLenum types[] = {
            GL_READ_ONLY, 
            GL_WRITE_ONLY, 
            GL_READ_WRITE
        };
        return types[static_cast<int>(type)];
    }

    static GLenum To(BarrierType type)
    {
        const GLenum types[] = {
            GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT, 
            GL_ELEMENT_ARRAY_BARRIER_BIT, 
            GL_UNIFORM_BARRIER_BIT, 
            GL_TEXTURE_FETCH_BARRIER_BIT, 
            GL_SHADER_IMAGE_ACCESS_BARRIER_BIT, 
            GL_COMMAND_BARRIER_BIT, 
            GL_PIXEL_BUFFER_BARRIER_BIT, 
            GL_TEXTURE_UPDATE_BARRIER_BIT, 
            GL_BUFFER_UPDATE_BARRIER_BIT, 
            GL_FRAMEBUFFER_BARRIER_BIT, 
            GL_TRANSFORM_FEEDBACK_BARRIER_BIT, 
            GL_ATOMIC_COUNTER_BARRIER_BIT, 
            GL_SHADER_STORAGE_BARRIER_BIT
        };
        return types[static_cast<int>(type)];
    }

}; // TypeConverter

}
}