#pragma once
#include <PCH.h>

namespace sy::vk
{
	enum class EAccessPattern
	{
		None,

		CommandBufferRead,
		IndirectBuffer,
		IndexBuffer,
		VertexBuffer,

		VertexShaderReadUniformBuffer,
		VertexShaderReadStorageBuffer,
		VertexShaderReadSampledImage,
		VertexShaderReadUniformTexelBuffer,
		VertexShaderReadGeneral,

		TessellationControlShaderReadUniformBuffer,
		TessellationControlShaderReadStorageBuffer,
		TessellationControlShaderReadSampledImage,
		TessellationControlShaderReadUniformTexelBuffer,
		TessellationControlShaderReadGeneral,

		TessellationEvaluationShaderReadUniformBuffer,
		TessellationEvaluationShaderReadStorageBuffer,
		TessellationEvaluationShaderReadSampledImage,
		TessellationEvaluationShaderReadUniformTexelBuffer,
		TessellationEvaluationShaderReadGeneral,

		GeometryShaderReadUniformBuffer,
		GeometryShaderReadStorageBuffer,
		GeometryShaderReadSampledImage,
		GeometryShaderReadUniformTexelBuffer,
		GeometryShaderReadGeneral,

		TaskShaderReadUniformBuffer,
		TaskShaderReadStorageBuffer,
		TaskShaderReadSampledImage,
		TaskShaderReadUniformTexelBuffer,
		TaskShaderReadGeneral,

		MeshShaderReadUniformBuffer,
		MeshShaderReadStorageBuffer,
		MeshShaderReadSampledImage,
		MeshShaderReadUniformTexelBuffer,
		MeshShaderReadGeneral,

		TransformFeedbackCounterRead,
		FragmentDensityMapRead,
		ShadingRateRead,

		FragmentShaderReadUniformBuffer,
		FragmentShaderReadStorageBuffer,
		FragmentShaderReadSampledImage,
		FragmentShaderReadUniformTexelBuffer,
		FragmentShaderReadColorInputAttachment,
		FragmentShaderReadDepthStencilInputAttachment,
		FragmentShaderReadGeneral,

		ColorAttachmentRead,
		ColorAttachmentAdvancedBlending,
		DepthStencilAttachmentRead,

		ComputeShaderReadUniformBuffer,
		ComputeShaderReadStorageBuffer,
		ComputeShaderReadSampledImage,
		ComputeShaderReadUniformTexelBuffer,
		ComputeShaderReadGeneral,

		AnyShaderReadUniformBuffer,
		AnyShaderReadStorageBuffer,
		AnyShaderReadUniformBufferOrVertexBuffer,
		AnyShaderReadSampledImage,
		AnyShaderReadUniformTexelBuffer,
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
		VkPipelineStageFlags2 PipelineStage;
		VkAccessFlags2 Access;
		VkImageLayout ImageLayout;
	};

	static AccessPattern QueryAccessPattern(const EAccessPattern pattern)
	{
		switch (pattern)
		{
		case EAccessPattern::None:
			return { 0, 0, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::CommandBufferRead:
			return { VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV, VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_NV, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::IndirectBuffer:
			return { VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::IndexBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::VertexBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::VertexShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::VertexShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::VertexShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::VertexShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::VertexShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TessellationControlShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::TessellationControlShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::TessellationControlShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::TessellationControlShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::TessellationControlShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TessellationEvaluationShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::TessellationEvaluationShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::TessellationEvaluationShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::TessellationEvaluationShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::TessellationEvaluationShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::GeometryShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::GeometryShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::GeometryShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::GeometryShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::GeometryShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TaskShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::TaskShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::TaskShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::TaskShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::TaskShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::MeshShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::MeshShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::MeshShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::MeshShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::MeshShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TransformFeedbackCounterRead:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::FragmentDensityMapRead:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT, VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT };

		case EAccessPattern::ShadingRateRead:
			return { VK_PIPELINE_STAGE_2_SHADING_RATE_IMAGE_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV };

		case EAccessPattern::FragmentShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::FragmentShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::FragmentShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::FragmentShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::FragmentShaderReadColorInputAttachment:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::FragmentShaderReadDepthStencilInputAttachment:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };

		case EAccessPattern::FragmentShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::ColorAttachmentRead:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case EAccessPattern::ColorAttachmentAdvancedBlending:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case EAccessPattern::DepthStencilAttachmentRead:
			return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };

		case EAccessPattern::ComputeShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::ComputeShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::ComputeShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::ComputeShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::ComputeShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::AnyShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::AnyShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::AnyShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EAccessPattern::AnyShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TransferRead:
			return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL };

		case EAccessPattern::HostRead:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::Present:
			return { 0, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };

		case EAccessPattern::ConditionalRenderingRead:
			return { VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT, VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::RayTracingShaderAccelerationStructureRead:
			return { VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::AccelerationStructureBuildRead:
			return { VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::EndOfRead:
			return { 0, 0, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::CommandBufferWrite:
			return { VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV, VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::VertexShaderWrite:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TessellationControlShaderWrite:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TessellationEvaluationShaderWrite:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::GeometryShaderWrite:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TaskShaderWrite:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::MeshShaderWrite:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TransformFeedbackWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::TransformFeedbackCounterWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::FragmentShaderWrite:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::ColorAttachmentWrite:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case EAccessPattern::DepthStencilAttachmentWrite:
			return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		case EAccessPattern::DepthAttachmentWriteStencilReadOnly:
			return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR };

		case EAccessPattern::StencilAttachmentWriteDepthReadOnly:
			return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR };

		case EAccessPattern::ComputeShaderWrite:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::AnyShaderWrite:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::TransferWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL };

		case EAccessPattern::HostPreInitialized:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED };

		case EAccessPattern::HostWrite:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EAccessPattern::AccelerationStructureBuildWrite:
			return { VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case EAccessPattern::ColorAttachmentReadWrite:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case EAccessPattern::General:
		default:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };
		}
	}
}