#pragma once
#include <PCH.h>

namespace sy::vk
{
	enum class EBufferAccessPattern
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

	enum class ETextureAccessPattern
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
		VkPipelineStageFlags2 PipelineStage;
		VkAccessFlags2 Access;
		VkImageLayout ImageLayout;
	};

	static AccessPattern QueryAccessPattern(const ETextureAccessPattern pattern)
	{
		switch (pattern)
		{
		case ETextureAccessPattern::None:
			return { 0, 0, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::VertexShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::VertexShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TessellationControlShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::TessellationControlShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TessellationEvaluationShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::TessellationEvaluationShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::GeometryShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::GeometryShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TaskShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::TaskShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::MeshShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::MeshShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TransformFeedbackCounterRead:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::FragmentDensityMapRead:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT, VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT };

		case ETextureAccessPattern::ShadingRateRead:
			return { VK_PIPELINE_STAGE_2_SHADING_RATE_IMAGE_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV };

		case ETextureAccessPattern::FragmentShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::FragmentShaderReadColorInputAttachment:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::FragmentShaderReadDepthStencilInputAttachment:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::FragmentShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::ColorAttachmentRead:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case ETextureAccessPattern::ColorAttachmentAdvancedBlending:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case ETextureAccessPattern::DepthStencilAttachmentRead:
			return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::ComputeShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::ComputeShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::AnyShaderReadSampledImage:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case ETextureAccessPattern::AnyShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TransferRead:
			return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL };

		case ETextureAccessPattern::HostRead:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::Present:
			return { 0, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };

		case ETextureAccessPattern::ConditionalRenderingRead:
			return { VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT, VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::RayTracingShaderAccelerationStructureRead:
			return { VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::AccelerationStructureBuildRead:
			return { VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::EndOfRead:
			return { 0, 0, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::CommandBufferWrite:
			return { VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV, VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::VertexShaderWrite:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TessellationControlShaderWrite:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TessellationEvaluationShaderWrite:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::GeometryShaderWrite:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TaskShaderWrite:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::MeshShaderWrite:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TransformFeedbackWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::TransformFeedbackCounterWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::FragmentShaderWrite:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::ColorAttachmentWrite:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case ETextureAccessPattern::DepthStencilAttachmentWrite:
			return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		case ETextureAccessPattern::DepthAttachmentWriteStencilReadOnly:
			return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR };

		case ETextureAccessPattern::StencilAttachmentWriteDepthReadOnly:
			return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR };

		case ETextureAccessPattern::ComputeShaderWrite:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::AnyShaderWrite:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::TransferWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL };

		case ETextureAccessPattern::HostPreInitialized:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED };

		case ETextureAccessPattern::HostWrite:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case ETextureAccessPattern::AccelerationStructureBuildWrite:
			return { VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case ETextureAccessPattern::ColorAttachmentReadWrite:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case ETextureAccessPattern::General:
		default:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };
		}
	}

	static AccessPattern QueryAccessPattern(const EBufferAccessPattern pattern)
	{
		switch (pattern)
		{
		case EBufferAccessPattern::None:
			return { 0, 0, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::CommandBufferRead:
			return { VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV, VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_NV, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::IndirectBuffer:
			return { VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::IndexBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::VertexBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::VertexShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::VertexShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::VertexShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::VertexShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TessellationControlShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::TessellationControlShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::TessellationControlShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::TessellationControlShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TessellationEvaluationShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::TessellationEvaluationShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::TessellationEvaluationShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::TessellationEvaluationShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::GeometryShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::GeometryShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::GeometryShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::GeometryShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TaskShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::TaskShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::TaskShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::TaskShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::MeshShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::MeshShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::MeshShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::MeshShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TransformFeedbackCounterRead:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::FragmentDensityMapRead:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_DENSITY_PROCESS_BIT_EXT, VK_ACCESS_2_FRAGMENT_DENSITY_MAP_READ_BIT_EXT, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT };

		case EBufferAccessPattern::ShadingRateRead:
			return { VK_PIPELINE_STAGE_2_SHADING_RATE_IMAGE_BIT_NV, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV };

		case EBufferAccessPattern::FragmentShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::FragmentShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::FragmentShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::FragmentShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::ComputeShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::ComputeShaderReadStorageBuffer:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::ComputeShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::ComputeShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::AnyShaderReadUniformBuffer:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::AnyShaderReadUniformTexelBuffer:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		case EBufferAccessPattern::AnyShaderReadGeneral:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TransferRead:
			return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL };

		case EBufferAccessPattern::HostRead:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::ConditionalRenderingRead:
			return { VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT, VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::RayTracingShaderAccelerationStructureRead:
			return { VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::AccelerationStructureBuildRead:
			return { VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::EndOfRead:
			return { 0, 0, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::CommandBufferWrite:
			return { VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV, VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_NV, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::VertexShaderWrite:
			return { VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TessellationControlShaderWrite:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TessellationEvaluationShaderWrite:
			return { VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::GeometryShaderWrite:
			return { VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TaskShaderWrite:
			return { VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::MeshShaderWrite:
			return { VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TransformFeedbackWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::TransformFeedbackCounterWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::FragmentShaderWrite:
			return { VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::ComputeShaderWrite:
			return { VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::AnyShaderWrite:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::TransferWrite:
			return { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL };

		case EBufferAccessPattern::HostPreInitialized:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED };

		case EBufferAccessPattern::HostWrite:
			return { VK_PIPELINE_STAGE_2_HOST_BIT, VK_ACCESS_2_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

		case EBufferAccessPattern::AccelerationStructureBuildWrite:
			return { VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

		case EBufferAccessPattern::ColorAttachmentReadWrite:
			return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		case EBufferAccessPattern::General:
		default:
			return { VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };
		}
	}
}