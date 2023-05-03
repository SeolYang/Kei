#pragma once
#include <PCH.h>

namespace sy::vk
{
enum class EBufferState
{
    None,

    CommandBufferRead,
    IndirectBuffer,
    IndexBuffer,
    VertexBuffer,

    VertexShaderReadUniformBuffer,
    VertexShaderReadStorageBuffer,
    VertexShaderReadUniformTexelBuffer,
    VertexShaderReadGeneral,

    TessellationControlShaderReadUniformBuffer,
    TessellationControlShaderReadStorageBuffer,
    TessellationControlShaderReadUniformTexelBuffer,
    TessellationControlShaderReadGeneral,

    TessellationEvaluationShaderReadUniformBuffer,
    TessellationEvaluationShaderReadStorageBuffer,
    TessellationEvaluationShaderReadUniformTexelBuffer,
    TessellationEvaluationShaderReadGeneral,

    GeometryShaderReadUniformBuffer,
    GeometryShaderReadStorageBuffer,
    GeometryShaderReadUniformTexelBuffer,
    GeometryShaderReadGeneral,

    TaskShaderReadUniformBuffer,
    TaskShaderReadStorageBuffer,
    TaskShaderReadUniformTexelBuffer,
    TaskShaderReadGeneral,

    MeshShaderReadUniformBuffer,
    MeshShaderReadStorageBuffer,
    MeshShaderReadUniformTexelBuffer,
    MeshShaderReadGeneral,

    TransformFeedbackCounterRead,
    FragmentDensityMapRead,
    ShadingRateRead,

    FragmentShaderReadUniformBuffer,
    FragmentShaderReadStorageBuffer,
    FragmentShaderReadUniformTexelBuffer,
    FragmentShaderReadGeneral,

    ComputeShaderReadUniformBuffer,
    ComputeShaderReadStorageBuffer,
    ComputeShaderReadUniformTexelBuffer,
    ComputeShaderReadGeneral,

    AnyShaderReadUniformBuffer,
    AnyShaderReadStorageBuffer,
    AnyShaderReadUniformBufferOrVertexBuffer,
    AnyShaderReadUniformTexelBuffer,
    AnyShaderReadGeneral,

    TransferRead,
    HostRead,

    ConditionalRenderingRead,
    RayTracingShaderAccelerationStructureRead,
    AccelerationStructureBuildRead,
    EndOfRead,

    CommandBufferWrite,
    VertexShaderWrite,
    TessellationControlShaderWrite,
    TessellationEvaluationShaderWrite,
    GeometryShaderWrite,
    TaskShaderWrite,
    MeshShaderWrite,
    TransformFeedbackWrite,
    TransformFeedbackCounterWrite,
    FragmentShaderWrite,
    ComputeShaderWrite,
    AnyShaderWrite,
    TransferWrite,

    HostPreInitialized,
    HostWrite,
    AccelerationStructureBuildWrite,
    ColorAttachmentReadWrite,

    General
};

enum class ETextureState
{
    None,

    VertexShaderReadSampledImage,
    VertexShaderReadGeneral,

    TessellationControlShaderReadSampledImage,
    TessellationControlShaderReadGeneral,

    TessellationEvaluationShaderReadSampledImage,
    TessellationEvaluationShaderReadGeneral,

    GeometryShaderReadSampledImage,
    GeometryShaderReadGeneral,

    TaskShaderReadSampledImage,
    TaskShaderReadGeneral,

    MeshShaderReadSampledImage,
    MeshShaderReadGeneral,

    TransformFeedbackCounterRead,
    FragmentDensityMapRead,
    ShadingRateRead,

    FragmentShaderReadSampledImage,
    FragmentShaderReadColorInputAttachment,
    FragmentShaderReadDepthStencilInputAttachment,
    FragmentShaderReadGeneral,

    ColorAttachmentRead,
    ColorAttachmentAdvancedBlending,
    DepthStencilAttachmentRead,

    ComputeShaderReadSampledImage,
    ComputeShaderReadGeneral,

    AnyShaderReadSampledImage,
    AnyShaderReadGeneral,

    TransferRead,
    HostRead,
    Present,

    ConditionalRenderingRead,
    RayTracingShaderAccelerationStructureRead,
    AccelerationStructureBuildRead,
    EndOfRead,

    CommandBufferWrite,
    VertexShaderWrite,
    TessellationControlShaderWrite,
    TessellationEvaluationShaderWrite,
    GeometryShaderWrite,
    TaskShaderWrite,
    MeshShaderWrite,
    TransformFeedbackWrite,
    TransformFeedbackCounterWrite,
    FragmentShaderWrite,
    ColorAttachmentWrite,
    DepthStencilAttachmentWrite,
    DepthAttachmentWriteStencilReadOnly,
    StencilAttachmentWriteDepthReadOnly,
    ComputeShaderWrite,
    AnyShaderWrite,
    TransferWrite,

    HostPreInitialized,
    HostWrite,
    AccelerationStructureBuildWrite,
    ColorAttachmentReadWrite,

    General
};

struct AccessPattern
{
public:
    void Overlap(const AccessPattern& rhs)
    {
        PipelineStage |= rhs.PipelineStage;
        Access |= rhs.Access;
        ImageLayout = ImageLayout != rhs.ImageLayout ? VK_IMAGE_LAYOUT_GENERAL : ImageLayout;
    }

public:
    VkPipelineStageFlags2 PipelineStage;
    VkAccessFlags2 Access;
    VkImageLayout ImageLayout;
};

static AccessPattern QueryAccessPattern(const ETextureState pattern)
{
    switch (pattern)
    {
        case ETextureState::None:
            return {0, 0, VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::VertexShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::VertexShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TessellationControlShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::TessellationControlShaderReadGeneral:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TessellationEvaluationShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::TessellationEvaluationShaderReadGeneral:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::GeometryShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::GeometryShaderReadGeneral:
            return {
                VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TaskShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::TaskShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::MeshShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::MeshShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TransformFeedbackCounterRead:
            return {
                VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
                VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::FragmentDensityMapRead:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
                VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
                VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

        case ETextureState::ShadingRateRead:
            return {
                VK_PIPELINE_STAGE_2_SHADING_RATE_IMAGE_BIT_NV,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV};

        case ETextureState::FragmentShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::FragmentShaderReadColorInputAttachment:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::FragmentShaderReadDepthStencilInputAttachment:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

        case ETextureState::FragmentShaderReadGeneral:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::ColorAttachmentRead:
            return {
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        case ETextureState::ColorAttachmentAdvancedBlending:
            return {
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        case ETextureState::DepthStencilAttachmentRead:
            return {
                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

        case ETextureState::ComputeShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::ComputeShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::AnyShaderReadSampledImage:
            return {
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case ETextureState::AnyShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TransferRead:
            return {
                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL};

        case ETextureState::HostRead:
            return {VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::Present:
            return {0, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

        case ETextureState::ConditionalRenderingRead:
            return {
                VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT,
                VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::RayTracingShaderAccelerationStructureRead:
            return {
                VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::AccelerationStructureBuildRead:
            return {
                VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::EndOfRead:
            return {0, 0, VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::CommandBufferWrite:
            return {
                VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV,
                VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::VertexShaderWrite:
            return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TessellationControlShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TessellationEvaluationShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::GeometryShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TaskShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::MeshShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TransformFeedbackWrite:
            return {
                VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
                VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::TransformFeedbackCounterWrite:
            return {
                VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
                VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::FragmentShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::ColorAttachmentWrite:
            return {
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        case ETextureState::DepthStencilAttachmentWrite:
            return {
                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        case ETextureState::DepthAttachmentWriteStencilReadOnly:
            return {
                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR};

        case ETextureState::StencilAttachmentWriteDepthReadOnly:
            return {
                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR};

        case ETextureState::ComputeShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::AnyShaderWrite:
            return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::TransferWrite:
            return {
                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};

        case ETextureState::HostPreInitialized:
            return {VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED};

        case ETextureState::HostWrite:
            return {VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case ETextureState::AccelerationStructureBuildWrite:
            return {
                VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case ETextureState::ColorAttachmentReadWrite:
            return {
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        case ETextureState::General:
        default:
            return {
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};
    }
}

static AccessPattern QueryAccessPattern(const EBufferState pattern)
{
    switch (pattern)
    {
        case EBufferState::None:
            return {0, 0, VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::CommandBufferRead:
            return {
                VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV,
                VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_NV,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::IndirectBuffer:
            return {
                VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
                VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::IndexBuffer:
            return {VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::VertexBuffer:
            return {
                VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
                VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::VertexShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::VertexShaderReadStorageBuffer:
            return {
                VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::VertexShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::VertexShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TessellationControlShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::TessellationControlShaderReadStorageBuffer:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::TessellationControlShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::TessellationControlShaderReadGeneral:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TessellationEvaluationShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::TessellationEvaluationShaderReadStorageBuffer:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::TessellationEvaluationShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::TessellationEvaluationShaderReadGeneral:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::GeometryShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::GeometryShaderReadStorageBuffer:
            return {
                VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::GeometryShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::GeometryShaderReadGeneral:
            return {
                VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TaskShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::TaskShaderReadStorageBuffer:
            return {
                VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::TaskShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::TaskShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::MeshShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::MeshShaderReadStorageBuffer:
            return {
                VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::MeshShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::MeshShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TransformFeedbackCounterRead:
            return {
                VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
                VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::FragmentDensityMapRead:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
                VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
                VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

        case EBufferState::ShadingRateRead:
            return {
                VK_PIPELINE_STAGE_2_SHADING_RATE_IMAGE_BIT_NV,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV};

        case EBufferState::FragmentShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::FragmentShaderReadStorageBuffer:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::FragmentShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::FragmentShaderReadGeneral:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::ComputeShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::ComputeShaderReadStorageBuffer:
            return {
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::ComputeShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::ComputeShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::AnyShaderReadUniformBuffer:
            return {
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_UNIFORM_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::AnyShaderReadUniformTexelBuffer:
            return {
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

        case EBufferState::AnyShaderReadGeneral:
            return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TransferRead:
            return {
                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL};

        case EBufferState::HostRead:
            return {VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_READ_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::ConditionalRenderingRead:
            return {
                VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT,
                VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::RayTracingShaderAccelerationStructureRead:
            return {
                VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::AccelerationStructureBuildRead:
            return {
                VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::EndOfRead:
            return {0, 0, VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::CommandBufferWrite:
            return {
                VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV,
                VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::VertexShaderWrite:
            return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TessellationControlShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TessellationEvaluationShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::GeometryShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TaskShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::MeshShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TransformFeedbackWrite:
            return {
                VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
                VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::TransformFeedbackCounterWrite:
            return {
                VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
                VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::FragmentShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::ComputeShaderWrite:
            return {
                VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::AnyShaderWrite:
            return {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::TransferWrite:
            return {
                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};

        case EBufferState::HostPreInitialized:
            return {VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED};

        case EBufferState::HostWrite:
            return {VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL};

        case EBufferState::AccelerationStructureBuildWrite:
            return {
                VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
                VK_IMAGE_LAYOUT_UNDEFINED};

        case EBufferState::ColorAttachmentReadWrite:
            return {
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        case EBufferState::General:
        default:
            return {
                VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
                VK_IMAGE_LAYOUT_GENERAL};
    }
}
} // namespace sy::vk
