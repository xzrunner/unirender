#include "unirender/vulkan/Pipeline.h"
#include "unirender/vulkan/PipelineLayout.h"
#include "unirender/vulkan/RenderPass.h"
#include "unirender/vulkan/PipelineCache.h"
#include "unirender/vulkan/VertexBuffer.h"
#include "unirender/vulkan/ShaderProgram.h"
#include "unirender/vulkan/Context.h"
#include "unirender/vulkan/LogicalDevice.h"

#include <assert.h>
#include <iostream>
#include <vector>

/* Number of samples needs to be the same at image creation,      */
/* renderpass creation and pipeline creation.                     */
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

/* Number of viewports and number of scissors have to be the same */
/* at pipeline creation and in any call to set them dynamically   */
/* They also have to be the same as each other                    */
#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

namespace
{

VkBlendFactor ToVkBlendFactor(ur::BlendingFactor f)
{
    switch (f)
    {
    case ur::BlendingFactor::Zero:                 return VK_BLEND_FACTOR_ZERO;
    case ur::BlendingFactor::One:                  return VK_BLEND_FACTOR_ONE;
    case ur::BlendingFactor::SrcColor:             return VK_BLEND_FACTOR_SRC_COLOR;
    case ur::BlendingFactor::OneMinusSrcColor:     return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case ur::BlendingFactor::DstColor:             return VK_BLEND_FACTOR_DST_COLOR;
    case ur::BlendingFactor::OneMinusDstColor:     return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case ur::BlendingFactor::SrcAlpha:             return VK_BLEND_FACTOR_SRC_ALPHA;
    case ur::BlendingFactor::OneMinusSrcAlpha:     return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case ur::BlendingFactor::DstAlpha:             return VK_BLEND_FACTOR_DST_ALPHA;
    case ur::BlendingFactor::OneMinusDstAlpha:     return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case ur::BlendingFactor::ConstantColor:        return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case ur::BlendingFactor::OneMinusConstantColor:return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case ur::BlendingFactor::ConstantAlpha:        return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case ur::BlendingFactor::OneMinusConstantAlpha:return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case ur::BlendingFactor::SrcAlphaSaturate:     return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    default:                                       return VK_BLEND_FACTOR_ONE;
    }
}

VkBlendOp ToVkBlendOp(ur::BlendEquation e)
{
    switch (e)
    {
    case ur::BlendEquation::Add:             return VK_BLEND_OP_ADD;
    case ur::BlendEquation::Subtract:        return VK_BLEND_OP_SUBTRACT;
    case ur::BlendEquation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case ur::BlendEquation::Minimum:         return VK_BLEND_OP_MIN;
    case ur::BlendEquation::Maximum:         return VK_BLEND_OP_MAX;
    default:                                 return VK_BLEND_OP_ADD;
    }
}

}

namespace ur
{
namespace vulkan
{

Pipeline::Pipeline(const Context& ctx, bool include_depth, bool include_vi,
                   const ur::PipelineLayout& layout, const ur::VertexBuffer& vb,
                   const ur::ShaderProgram& prog)
    : m_device(ctx.GetLogicalDevice())
{
    Build(ctx, include_depth, include_vi, vb, prog, Blending(), VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
}

Pipeline::Pipeline(const Context& ctx, bool include_depth, bool include_vi,
                   const ur::VertexBuffer& vb, const ur::ShaderProgram& prog,
                   const Blending& blend, VkPrimitiveTopology topology)
    : m_device(ctx.GetLogicalDevice())
{
    Build(ctx, include_depth, include_vi, vb, prog, blend, topology);
}

void Pipeline::Build(const Context& ctx, bool include_depth, bool include_vi,
                     const ur::VertexBuffer& vb, const ur::ShaderProgram& prog,
                     const Blending& blend, VkPrimitiveTopology topology)
{
    VkResult res;

    VkDynamicState dynamicStateEnables[2];  // Viewport + Scissor
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;

    VkPipelineVertexInputStateCreateInfo vi;
    memset(&vi, 0, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (include_vi) 
    {
        auto& vert_buf = static_cast<const vulkan::VertexBuffer&>(vb);

        vi.pNext = NULL;
        vi.flags = 0;
        vi.vertexBindingDescriptionCount = 1;
        vi.pVertexBindingDescriptions = &vert_buf.GetVertInputBindDesc();

        auto& attrs = vert_buf.GetVertInputAttrDesc();
        vi.vertexAttributeDescriptionCount = attrs.size();
        vi.pVertexAttributeDescriptions = attrs.data();
    }
    VkPipelineInputAssemblyStateCreateInfo ia;
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.pNext = NULL;
    ia.flags = 0;
    ia.primitiveRestartEnable = VK_FALSE;
    ia.topology = topology;

    VkPipelineRasterizationStateCreateInfo rs;
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.pNext = NULL;
    rs.flags = 0;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = /*VK_CULL_MODE_BACK_BIT*/VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.depthBiasConstantFactor = 0;
    rs.depthBiasClamp = 0;
    rs.depthBiasSlopeFactor = 0;
    rs.lineWidth = 1.0f;

    VkPipelineColorBlendStateCreateInfo cb;
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.flags = 0;
    cb.pNext = NULL;
    // Honor the engine's RenderState.blending (was hardcoded OFF, which left the 2D
    // GUI -- DefaultRenderState2D wants SrcAlpha/OneMinusSrcAlpha -- compositing glyph
    // quads as opaque black boxes once the atlas had real content).
    VkPipelineColorBlendAttachmentState one = {};
    one.colorWriteMask = 0xf;
    if (blend.enabled)
    {
        one.blendEnable = VK_TRUE;
        if (blend.separately)
        {
            one.srcColorBlendFactor = ToVkBlendFactor(blend.src_rgb);
            one.dstColorBlendFactor = ToVkBlendFactor(blend.dst_rgb);
            one.srcAlphaBlendFactor = ToVkBlendFactor(blend.src_alpha);
            one.dstAlphaBlendFactor = ToVkBlendFactor(blend.dst_alpha);
            one.colorBlendOp        = ToVkBlendOp(blend.rgb_equation);
            one.alphaBlendOp        = ToVkBlendOp(blend.alpha_equation);
        }
        else
        {
            one.srcColorBlendFactor = ToVkBlendFactor(blend.src);
            one.dstColorBlendFactor = ToVkBlendFactor(blend.dst);
            one.srcAlphaBlendFactor = ToVkBlendFactor(blend.src);
            one.dstAlphaBlendFactor = ToVkBlendFactor(blend.dst);
            one.colorBlendOp        = ToVkBlendOp(blend.equation);
            one.alphaBlendOp        = ToVkBlendOp(blend.equation);
        }
    }
    else
    {
        one.blendEnable = VK_FALSE;
        one.alphaBlendOp = VK_BLEND_OP_ADD;
        one.colorBlendOp = VK_BLEND_OP_ADD;
        one.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        one.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        one.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        one.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    }
    // One blend state per color attachment -- the count MUST match the render pass
    // (MRT GBuffer has 3 color attachments; screen has 1). All attachments share the
    // same blend config here.
    uint32_t color_count = static_cast<const vulkan::Context&>(ctx).GetCurrentColorAttachmentCount();
    if (color_count == 0) { color_count = 1; }
    std::vector<VkPipelineColorBlendAttachmentState> att_states(color_count, one);
    cb.attachmentCount = color_count;
    cb.pAttachments = att_states.data();
    cb.logicOpEnable = VK_FALSE;
    cb.logicOp = VK_LOGIC_OP_NO_OP;
    cb.blendConstants[0] = 1.0f;
    cb.blendConstants[1] = 1.0f;
    cb.blendConstants[2] = 1.0f;
    cb.blendConstants[3] = 1.0f;

    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.pNext = NULL;
    vp.flags = 0;
#ifndef __ANDROID__
    vp.viewportCount = NUM_VIEWPORTS;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    vp.scissorCount = NUM_SCISSORS;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
    vp.pScissors = NULL;
    vp.pViewports = NULL;
#else
    // Temporary disabling dynamic viewport on Android because some of drivers doesn't
    // support the feature.
    VkViewport viewports;
    viewports.minDepth = 0.0f;
    viewports.maxDepth = 1.0f;
    viewports.x = 0;
    viewports.y = 0;
    viewports.width = info.width;
    viewports.height = info.height;
    VkRect2D scissor;
    scissor.extent.width = info.width;
    scissor.extent.height = info.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vp.viewportCount = NUM_VIEWPORTS;
    vp.scissorCount = NUM_SCISSORS;
    vp.pScissors = &scissor;
    vp.pViewports = &viewports;
#endif
    VkPipelineDepthStencilStateCreateInfo ds;
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = NULL;
    ds.flags = 0;
    ds.depthTestEnable = include_depth;
    ds.depthWriteEnable = include_depth;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.stencilTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.back.compareMask = 0;
    ds.back.reference = 0;
    ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
    ds.back.writeMask = 0;
    ds.minDepthBounds = 0;
    ds.maxDepthBounds = 0;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    VkPipelineMultisampleStateCreateInfo ms;
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pNext = NULL;
    ms.flags = 0;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = NUM_SAMPLES;
    ms.sampleShadingEnable = VK_FALSE;
    ms.alphaToCoverageEnable = VK_FALSE;
    ms.alphaToOneEnable = VK_FALSE;
    ms.minSampleShading = 0.0;

    VkGraphicsPipelineCreateInfo pipeline;
    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.pNext = NULL;
    // Use the shader program's reflection-built pipeline layout so the pipeline's
    // descriptor expectations always match the (binding-deconflicted) shader. The
    // `layout` param (the engine's hardcoded single_ubo_single_img) is ignored on
    // Vulkan now that the ShaderProgram owns the real layout.
    pipeline.layout = static_cast<const vulkan::ShaderProgram&>(prog).GetVkPipelineLayout();
    pipeline.basePipelineHandle = VK_NULL_HANDLE;
    pipeline.basePipelineIndex = 0;
    pipeline.flags = 0;
    pipeline.pVertexInputState = &vi;
    pipeline.pInputAssemblyState = &ia;
    pipeline.pRasterizationState = &rs;
    pipeline.pColorBlendState = &cb;
    pipeline.pTessellationState = NULL;
    pipeline.pMultisampleState = &ms;
    pipeline.pDynamicState = &dynamicState;
    pipeline.pViewportState = &vp;
    pipeline.pDepthStencilState = &ds;
    pipeline.pStages = static_cast<const vulkan::ShaderProgram&>(prog).GetShaderStages();
    pipeline.stageCount = 2;
    // Build against the pass currently being recorded (screen or offscreen) so the
    // pipeline is render-pass-compatible with where it's used; fall back to the
    // screen pass if built outside a pass.
    {
        VkRenderPass cur = static_cast<const vulkan::Context&>(ctx).GetCurrentRenderPass();
        pipeline.renderPass = cur != VK_NULL_HANDLE ? cur : ctx.GetRenderPass()->GetHandler();
    }
    pipeline.subpass = 0;

    res = vkCreateGraphicsPipelines(m_device->GetHandler(), ctx.GetPipelineCache()->GetHandler(), 1, &pipeline, NULL, &m_handle);
    if (res != VK_SUCCESS) {
        // Don't abort the whole app: one shader with a bad VS->FS interface (e.g. the
        // 3D deferred mesh) must not take down the 2D GUI. Leave m_handle null; callers
        // (Context::RecordDraw) skip the draw when the handle is null.
        std::cerr << "[Vulkan] pipeline build failed (VkResult " << res << "); draw skipped" << std::endl;
        m_handle = VK_NULL_HANDLE;
    }
}

Pipeline::~Pipeline()
{
    if (m_handle != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->GetHandler(), m_handle, nullptr);
    }
}

}
}