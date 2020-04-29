#pragma once

namespace ur
{

enum class BufferTarget
{
    ArrayBuffer,
    ElementArrayBuffer,
    PixelPackBuffer,
    PixelUnpackBuffer,
    UniformBuffer,
    TextureBuffer,
    TransformFeedbackBuffer,
    CopyReadBuffer,
    CopyWriteBuffer,
    ShaderStorageBuffer,

}; // BufferTarget

}