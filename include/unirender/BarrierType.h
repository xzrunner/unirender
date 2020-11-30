#pragma once

namespace ur
{

enum class BarrierType
{
    VertexAttribArray,
    ElementArray,
    Uniform,
    TextureFetch,
    ShaderImageAccess,
    Command,
    PixelBuffer,
    TextureUpdate,
    BufferUpdate,
    Framebuffer,
    TransformFeedback,
    AtomicCounter,
    ShaderStorage
};

}