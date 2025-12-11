cmake_minimum_required(VERSION 3.16)

project(unirender)

add_definitions(
    -DNO_BOOST
    -DVK_USE_PLATFORM_WIN32_KHR
)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "include/unirender/ClearState.h"
    "include/unirender/Context.h"
    "include/unirender/Device.h"
    "include/unirender/DrawState.h"
    "include/unirender/Factory.h"
    "include/unirender/typedef.h"
    "source/Device.cpp"
    "source/Factory.cpp"
)
source_group("" FILES ${no_group_source_files})

set(api__opengl
    "include/unirender/opengl/Context.h"
    "include/unirender/opengl/Device.h"
    "include/unirender/opengl/opengl.h"
    "source/opengl/Context.cpp"
    "source/opengl/Device.cpp"
)
source_group("api\\opengl" FILES ${api__opengl})

set(api__opengl__buffer
    "include/unirender/opengl/Buffer.h"
    "include/unirender/opengl/ComputeBuffer.h"
    "include/unirender/opengl/IndexBuffer.h"
    "include/unirender/opengl/PixelBuffer.h"
    "include/unirender/opengl/ReadPixelBuffer.h"
    "include/unirender/opengl/RenderBuffer.h"
    "include/unirender/opengl/StorageBuffer.h"
    "include/unirender/opengl/TextureBuffer.h"
    "include/unirender/opengl/VertexBuffer.h"
    "include/unirender/opengl/WritePixelBuffer.h"
    "source/opengl/Buffer.cpp"
    "source/opengl/ComputeBuffer.cpp"
    "source/opengl/IndexBuffer.cpp"
    "source/opengl/PixelBuffer.cpp"
    "source/opengl/ReadPixelBuffer.cpp"
    "source/opengl/RenderBuffer.cpp"
    "source/opengl/StorageBuffer.cpp"
    "source/opengl/TextureBuffer.cpp"
    "source/opengl/VertexBuffer.cpp"
    "source/opengl/WritePixelBuffer.cpp"
)
source_group("api\\opengl\\buffer" FILES ${api__opengl__buffer})

set(api__opengl__frame_buffer
    "include/unirender/opengl/Framebuffer.h"
    "source/opengl/Framebuffer.cpp"
)
source_group("api\\opengl\\frame_buffer" FILES ${api__opengl__frame_buffer})

set(api__opengl__image
    "include/unirender/opengl/ImageUnit.h"
    "include/unirender/opengl/ImageUnits.h"
    "source/opengl/ImageUnit.cpp"
    "source/opengl/ImageUnits.cpp"
)
source_group("api\\opengl\\image" FILES ${api__opengl__image})

set(api__opengl__shader
    "include/unirender/opengl/ShaderObject.h"
    "include/unirender/opengl/ShaderProgram.h"
    "include/unirender/opengl/Uniform.h"
    "include/unirender/opengl/Uniform.inl"
    "source/opengl/ShaderObject.cpp"
    "source/opengl/ShaderProgram.cpp"
)
source_group("api\\opengl\\shader" FILES ${api__opengl__shader})

set(api__opengl__texture
    "include/unirender/opengl/Texture.h"
    "include/unirender/opengl/TextureFormat.h"
    "include/unirender/opengl/TextureSampler.h"
    "include/unirender/opengl/TextureUnit.h"
    "include/unirender/opengl/TextureUnits.h"
    "source/opengl/Texture.cpp"
    "source/opengl/TextureFormat.cpp"
    "source/opengl/TextureSampler.cpp"
    "source/opengl/TextureUnit.cpp"
    "source/opengl/TextureUnits.cpp"
)
source_group("api\\opengl\\texture" FILES ${api__opengl__texture})

set(api__opengl__utility
    "include/unirender/opengl/TypeConverter.h"
)
source_group("api\\opengl\\utility" FILES ${api__opengl__utility})

set(api__opengl__vertex_array
    "include/unirender/opengl/VertexArray.h"
    "include/unirender/opengl/VertexInputAttributes.h"
    "source/opengl/VertexArray.cpp"
    "source/opengl/VertexInputAttributes.cpp"
)
source_group("api\\opengl\\vertex_array" FILES ${api__opengl__vertex_array})

set(api__vulkan
    "include/unirender/vulkan/Context.h"
    "include/unirender/vulkan/Device.h"
    "source/vulkan/Context.cpp"
    "source/vulkan/Device.cpp"
)
source_group("api\\vulkan" FILES ${api__vulkan})

set(api__vulkan__0.setup
    "include/unirender/vulkan/Instance.h"
    "include/unirender/vulkan/LogicalDevice.h"
    "include/unirender/vulkan/PhysicalDevice.h"
    "include/unirender/vulkan/ValidationLayers.h"
    "source/vulkan/Instance.cpp"
    "source/vulkan/LogicalDevice.cpp"
    "source/vulkan/PhysicalDevice.cpp"
    "source/vulkan/ValidationLayers.cpp"
)
source_group("api\\vulkan\\0.setup" FILES ${api__vulkan__0.setup})

set(api__vulkan__1.presentation
    "include/unirender/vulkan/Surface.h"
    "include/unirender/vulkan/Swapchain.h"
    "source/vulkan/Surface.cpp"
    "source/vulkan/Swapchain.cpp"
)
source_group("api\\vulkan\\1.presentation" FILES ${api__vulkan__1.presentation})

set(api__vulkan__2.pipeline
    "include/unirender/vulkan/Pipeline.h"
    "include/unirender/vulkan/PipelineCache.h"
    "include/unirender/vulkan/PipelineLayout.h"
    "include/unirender/vulkan/RenderPass.h"
    "include/unirender/vulkan/ShaderObject.h"
    "include/unirender/vulkan/ShaderProgram.h"
    "include/unirender/vulkan/VertexArray.h"
    "source/vulkan/Pipeline.cpp"
    "source/vulkan/PipelineCache.cpp"
    "source/vulkan/PipelineLayout.cpp"
    "source/vulkan/RenderPass.cpp"
    "source/vulkan/ShaderObject.cpp"
    "source/vulkan/ShaderProgram.cpp"
    "source/vulkan/VertexArray.cpp"
)
source_group("api\\vulkan\\2.pipeline" FILES ${api__vulkan__2.pipeline})

set(api__vulkan__3.drawing__command
    "include/unirender/vulkan/CommandBuffer.h"
    "include/unirender/vulkan/CommandPool.h"
    "source/vulkan/CommandBuffer.cpp"
    "source/vulkan/CommandPool.cpp"
)
source_group("api\\vulkan\\3.drawing\\command" FILES ${api__vulkan__3.drawing__command})

set(api__vulkan__3.drawing__image
    "include/unirender/vulkan/Image.h"
    "include/unirender/vulkan/ImageView.h"
    "include/unirender/vulkan/Texture.h"
    "include/unirender/vulkan/TextureSampler.h"
    "source/vulkan/Image.cpp"
    "source/vulkan/ImageView.cpp"
    "source/vulkan/Texture.cpp"
    "source/vulkan/TextureSampler.cpp"
)
source_group("api\\vulkan\\3.drawing\\image" FILES ${api__vulkan__3.drawing__image})

set(api__vulkan__3.drawing__pixels
    "include/unirender/vulkan/DepthBuffer.h"
    "include/unirender/vulkan/FrameBuffers.h"
    "source/vulkan/DepthBuffer.cpp"
    "source/vulkan/FrameBuffers.cpp"
)
source_group("api\\vulkan\\3.drawing\\pixels" FILES ${api__vulkan__3.drawing__pixels})

set(api__vulkan__3.drawing__uniform
    "include/unirender/vulkan/DescriptorPool.h"
    "include/unirender/vulkan/DescriptorSet.h"
    "include/unirender/vulkan/DescriptorSetLayout.h"
    "include/unirender/vulkan/UniformBuffer.h"
    "source/vulkan/DescriptorPool.cpp"
    "source/vulkan/DescriptorSet.cpp"
    "source/vulkan/DescriptorSetLayout.cpp"
    "source/vulkan/UniformBuffer.cpp"
)
source_group("api\\vulkan\\3.drawing\\uniform" FILES ${api__vulkan__3.drawing__uniform})

set(api__vulkan__3.drawing__vertex
    "include/unirender/vulkan/IndexBuffer.h"
    "include/unirender/vulkan/VertexBuffer.h"
    "source/vulkan/IndexBuffer.cpp"
    "source/vulkan/VertexBuffer.cpp"
)
source_group("api\\vulkan\\3.drawing\\vertex" FILES ${api__vulkan__3.drawing__vertex})

set(api__vulkan__9.utility
    "include/unirender/vulkan/Buffer.h"
    "include/unirender/vulkan/TypeConverter.h"
    "include/unirender/vulkan/Utility.h"
    "source/vulkan/Buffer.cpp"
    "source/vulkan/Utility.cpp"
)
source_group("api\\vulkan\\9.utility" FILES ${api__vulkan__9.utility})

set(buffer
    "include/unirender/BufferTarget.h"
    "include/unirender/BufferUsageHint.h"
    "include/unirender/ComputeBuffer.h"
    "include/unirender/IndexBuffer.h"
    "include/unirender/ReadPixelBuffer.h"
    "include/unirender/RenderBuffer.h"
    "include/unirender/StorageBuffer.h"
    "include/unirender/TextureBuffer.h"
    "include/unirender/VertexBuffer.h"
    "include/unirender/WritePixelBuffer.h"
)
source_group("buffer" FILES ${buffer})

set(frame_buffer
    "include/unirender/AttachmentType.h"
    "include/unirender/Framebuffer.h"
)
source_group("frame_buffer" FILES ${frame_buffer})

set(image
    "include/unirender/ImageFormat.h"
    "include/unirender/ImageLayout.h"
    "include/unirender/ImageUnit.h"
    "include/unirender/ImageView.h"
)
source_group("image" FILES ${image})

set(pipeline
    "include/unirender/Pipeline.h"
    "include/unirender/PipelineLayout.h"
    "include/unirender/RenderPass.h"
)
source_group("pipeline" FILES ${pipeline})

set(render_state
    "include/unirender/AlphaTest.h"
    "include/unirender/Blending.h"
    "include/unirender/ColorMask.h"
    "include/unirender/DepthRange.h"
    "include/unirender/DepthTest.h"
    "include/unirender/FacetCulling.h"
    "include/unirender/PrimitiveRestart.h"
    "include/unirender/RenderState.h"
    "include/unirender/ScissorTest.h"
    "include/unirender/StencilTest.h"
    "include/unirender/StencilTestFace.h"
)
source_group("render_state" FILES ${render_state})

set(shader
    "include/unirender/ShaderProgram.h"
    "include/unirender/ShaderType.h"
    "include/unirender/ShaderVertexAttribute.h"
    "include/unirender/Uniform.h"
    "include/unirender/UniformType.h"
    "include/unirender/UniformUpdater.h"
    "include/unirender/UniformValue.h"
    "source/ShaderProgram.cpp"
)
source_group("shader" FILES ${shader})

set(texture
    "include/unirender/InternalFormat.h"
    "include/unirender/Texture.h"
    "include/unirender/TextureDescription.h"
    "include/unirender/TextureFormat.h"
    "include/unirender/TextureMagnificationFilter.h"
    "include/unirender/TextureMinificationFilter.h"
    "include/unirender/TextureSampler.h"
    "include/unirender/TextureTarget.h"
    "include/unirender/TextureUnit.h"
    "include/unirender/TextureUtility.h"
    "include/unirender/TextureWrap.h"
    "source/TextureUtility.cpp"
)
source_group("texture" FILES ${texture})

set(uniform_buffer
    "include/unirender/Descriptor.h"
    "include/unirender/DescriptorPool.h"
    "include/unirender/DescriptorSet.h"
    "include/unirender/DescriptorSetLayout.h"
    "include/unirender/DescriptorType.h"
    "include/unirender/UniformBuffer.h"
)
source_group("uniform_buffer" FILES ${uniform_buffer})

set(utility
    "include/unirender/AccessType.h"
    "include/unirender/Adaptor.h"
    "include/unirender/BarrierType.h"
    "include/unirender/Bitmap.h"
    "include/unirender/Bitmap.inl"
    "include/unirender/Color.h"
    "include/unirender/noncopyable.h"
    "include/unirender/PrimitiveType.h"
    "include/unirender/Rectangle.h"
    "include/unirender/WindingOrder.h"
    "source/Adaptor.cpp"
)
source_group("utility" FILES ${utility})

set(vertex_array
    "include/unirender/ComponentDataType.h"
    "include/unirender/VertexArray.h"
    "include/unirender/VertexArraySizes.h"
    "include/unirender/VertexInputAttribute.h"
    "include/unirender/VertexLayoutType.h"
    "source/VertexInputAttribute.cpp"
)
source_group("vertex_array" FILES ${vertex_array})

set(ALL_FILES
    ${no_group_source_files}
    ${api__opengl}
    ${api__opengl__buffer}
    ${api__opengl__frame_buffer}
    ${api__opengl__image}
    ${api__opengl__shader}
    ${api__opengl__texture}
    ${api__opengl__utility}
    ${api__opengl__vertex_array}
    ${api__vulkan}
    ${api__vulkan__0.setup}
    ${api__vulkan__1.presentation}
    ${api__vulkan__2.pipeline}
    ${api__vulkan__3.drawing__command}
    ${api__vulkan__3.drawing__image}
    ${api__vulkan__3.drawing__pixels}
    ${api__vulkan__3.drawing__uniform}
    ${api__vulkan__3.drawing__vertex}
    ${api__vulkan__9.utility}
    ${buffer}
    ${frame_buffer}
    ${image}
    ${pipeline}
    ${render_state}
    ${shader}
    ${texture}
    ${uniform_buffer}
    ${utility}
    ${vertex_array}
)

add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE include external/Vulkan-Headers/include external/gl3w/include external/shadertrans/include)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif()
