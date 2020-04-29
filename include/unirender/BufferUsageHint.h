#pragma once

namespace ur
{

enum class BufferUsageHint
{
    StreamDraw,
    StreamRead,
    StreamCopy,
    StaticDraw,
    StaticRead,
    StaticCopy,
    DynamicDraw,
    DynamicRead,
    DynamicCopy,

}; // BufferUsageHint

}