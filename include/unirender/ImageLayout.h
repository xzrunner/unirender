#pragma once

namespace ur
{

enum class ImageLayout
{
    Undefined,
    General,
    ColorAttachmentOptimal,
    DepthStencilAttachmentOptimal,
    DepthStencilReadOnlyOptimal,
    ShaderReadOnlyOptimal,
    TransferSrcOptimal,
    TransferDstOptimal,
    Preinitialized,
    DepthReadOnlyStencilAttachmentOptimal,
    DepthAttachmentStencilReadOnlyOptimal,
    DepthAttachmentOptimal,
    DepthReadOnlyOptimal,
    StencilAttachmentOptimal,
    StencilReadOnlyOptimal,

    MaxCount

}; // ImageLayout

}