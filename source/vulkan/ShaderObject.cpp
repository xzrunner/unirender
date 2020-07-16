#define NOMINMAX 

#include "unirender/vulkan/ShaderObject.h"
#include "unirender/vulkan/TypeConverter.h"

#include <glslang/public/ShaderLang.h>
#include <glslang/include/StandAlone/DirStackFileIncluder.h>
#include <glslang/include/SPIRV/GlslangToSpv.h>

#include <iostream>

#include <assert.h>

namespace
{

const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    ///* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }
};

EShLanguage type2glslang(ur::ShaderType type)
{
    switch (type)
    {
    case ur::ShaderType::VertexShader:
        return EShLangVertex;
    case ur::ShaderType::FragmentShader:
        return EShLangFragment;
    case ur::ShaderType::GeometryShader:
        return EShLangGeometry;
    case ur::ShaderType::ComputeShader:
        return EShLangCompute;
    default:
        assert(0);
        return EShLangCount;
    }
}

}

namespace ur
{
namespace vulkan
{

ShaderObject::ShaderObject(VkDevice dev, ShaderType type, const std::vector<unsigned int>& spirv)
    : m_dev(dev)
{
    Init(type, spirv);
}

ShaderObject::ShaderObject(VkDevice dev, ShaderType type, const std::string& glsl)
    : m_dev(dev)
{
    std::vector<unsigned int> spirv;
    SpirvFromGLSL(type, glsl, spirv);
    if (!spirv.empty()) {
        Init(type, spirv);
    }
}

ShaderObject::~ShaderObject()
{
    vkDestroyShaderModule(m_dev, m_stage.module, NULL);
}

void ShaderObject::SpirvFromGLSL(ShaderType type, const std::string& glsl,
                                 std::vector<unsigned int>& spirv)
{
    const EShLanguage shader_type = type2glslang(type);
    glslang::TShader shader(shader_type);
    const char* src_cstr = glsl.c_str();
    shader.setStrings(&src_cstr, 1);
    
    int client_input_semantics_version = 100; // maps to, say, #define VULKAN 100
    glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0;
    glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_0;

    shader.setEnvInput(glslang::EShSourceGlsl, shader_type, glslang::EShClientVulkan, client_input_semantics_version);
    shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
    shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

    shader.setAutoMapLocations(true);
    shader.setAutoMapBindings(true);

    TBuiltInResource resources;
    resources = DefaultTBuiltInResource;
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    const int default_version = 100;

    DirStackFileIncluder includer;

    ////Get Path of File
    //std::string Path = GetFilePath(filename);
    //includer.pushExternalLocalDirectory(Path);

    std::string preprocessed_glsl;
    if (!shader.preprocess(&resources, default_version, ENoProfile, false, false, messages, &preprocessed_glsl, includer)) 
    {
        std::cout << "GLSL Preprocessing Failed for: \n" << glsl << std::endl;
        std::cout << shader.getInfoLog() << std::endl;
        std::cout << shader.getInfoDebugLog() << std::endl;
        return;
    }

    const char* preprocessed_cstr = preprocessed_glsl.c_str();
    shader.setStrings(&preprocessed_cstr, 1);

    if (!shader.parse(&resources, 100, false, messages))
    {
        std::cout << "GLSL Parsing Failed for: \n" << glsl << std::endl;
        std::cout << shader.getInfoLog() << std::endl;
        std::cout << shader.getInfoDebugLog() << std::endl;
        return;
    }

    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages))
    {
        std::cout << "GLSL Linking Failed for: \n" << glsl << std::endl;
        std::cout << shader.getInfoLog() << std::endl;
        std::cout << shader.getInfoDebugLog() << std::endl;
        return;
    }

    spv::SpvBuildLogger logger;
    glslang::SpvOptions spv_options;
    glslang::GlslangToSpv(*program.getIntermediate(shader_type), spirv, &logger, &spv_options);
}

void ShaderObject::Init(ShaderType type, const std::vector<unsigned int>& spirv)
{
    VkShaderModuleCreateInfo shader_ci{};
    shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_ci.pNext = nullptr;
    shader_ci.flags = 0;
    shader_ci.codeSize = spirv.size() * sizeof(uint32_t);
    shader_ci.pCode = spirv.data();

    m_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stage.pNext = NULL;
    m_stage.pSpecializationInfo = NULL;
    m_stage.flags = 0;
    m_stage.stage = TypeConverter::To(type);
    m_stage.pName = "main";
    VkResult res = vkCreateShaderModule(m_dev, &shader_ci, NULL, &m_stage.module);
    assert(res == VK_SUCCESS);
}

}
}