#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace vk
	{
		class Buffer;
		class Texture;
		class TextureView;

		constexpr uint32_t MaxBindlessResourcesPerDescriptor = 2048;
		constexpr size_t NumMaxInFlightFrames = 2;

		enum class EQueueType
		{
			Graphics,
			Compute,
			Transfer,
			Present
		};

		enum class EPipelineType
		{
			Graphics,
			Compute,
			RayTracing
		};

		enum class EDescriptorType : uint8_t
		{
			Sampler = 0,
			SampledImage,
			CombinedImageSampler,
			StorageImage,
			UniformBuffer,
			StorageBuffer,
			InputAttachment,
			UniformBufferDynamic,
			StorageBufferDynamic,
			EnumMax
		};

		constexpr static VkPipelineBindPoint ToNative(const EPipelineType type)
		{
			switch (type)
			{
			case EPipelineType::Compute:
				return VK_PIPELINE_BIND_POINT_COMPUTE;
			case EPipelineType::RayTracing:
				return VK_PIPELINE_BIND_POINT_RAY_TRACING_NV;
			default:
			case EPipelineType::Graphics:
				return VK_PIPELINE_BIND_POINT_GRAPHICS;
			}
		}

		constexpr static auto DescriptorTypeToBufferUsage(const EDescriptorType descriptorType)
		{
			switch (descriptorType)
			{

			case EDescriptorType::UniformBuffer:
			case EDescriptorType::UniformBufferDynamic:
				return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			case EDescriptorType::StorageBuffer:
			case EDescriptorType::StorageBufferDynamic:
				return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			default:
				SY_ASSERT(false, "Invalid value!");
				return static_cast<VkBufferUsageFlagBits>(0);
			}
		}

		constexpr static auto DescriptorTypeToImageUsage(const EDescriptorType descriptorType)
		{
			switch (descriptorType)
			{
			case EDescriptorType::SampledImage:
			case EDescriptorType::CombinedImageSampler:
				return VK_IMAGE_USAGE_SAMPLED_BIT;
			case EDescriptorType::StorageImage:
				return VK_IMAGE_USAGE_STORAGE_BIT;
			case EDescriptorType::InputAttachment:
				return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			default:
				SY_ASSERT(false, "Invalid value!");
				return static_cast<VkImageUsageFlagBits>(0);
			}
		}

		constexpr static auto BufferUsageToDescriptorType(const VkBufferUsageFlags bufferUsageFlags, bool bIsDynamic = false)
		{
			switch (bufferUsageFlags)
			{
			case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
				return bIsDynamic ? EDescriptorType::UniformBufferDynamic : EDescriptorType::UniformBuffer;
			case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
				return bIsDynamic ? EDescriptorType::StorageBufferDynamic : EDescriptorType::StorageBuffer;
			default:
				SY_ASSERT(false, "Invalid value!");
				return EDescriptorType::EnumMax;
			}
		}

		constexpr static auto ImageUsageToDescriptorType(const VkImageUsageFlags imageUsageFlags, const bool bIsCombinedSampler = true)
		{
			if ((imageUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) == VK_IMAGE_USAGE_STORAGE_BIT)
			{
				return EDescriptorType::StorageImage;
			}

			else if ((imageUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) == VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
			{
				return EDescriptorType::InputAttachment;
			}

			else if ((imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) == VK_IMAGE_USAGE_SAMPLED_BIT)
			{
				return bIsCombinedSampler ? EDescriptorType::CombinedImageSampler : EDescriptorType::SampledImage;
			}

			SY_ASSERT(false, "Unable to convert to desscriptor type.");
			return EDescriptorType::EnumMax;
		}

		constexpr static auto ToNative(const EDescriptorType descriptorType)
		{
			switch (descriptorType)
			{
			case EDescriptorType::Sampler:
				return VK_DESCRIPTOR_TYPE_SAMPLER;
			case EDescriptorType::SampledImage:
				return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case EDescriptorType::CombinedImageSampler:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case EDescriptorType::StorageImage:
				return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case EDescriptorType::UniformBuffer:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case EDescriptorType::UniformBufferDynamic:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			case EDescriptorType::StorageBuffer:
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case EDescriptorType::StorageBufferDynamic:
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			case EDescriptorType::InputAttachment:
				return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			default:
				SY_ASSERT(false, "Invalid value!");
				return VK_DESCRIPTOR_TYPE_MAX_ENUM;
			}
		}

		constexpr static size_t ToByteSize(const VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_R4G4_UNORM_PACK8:
			case VK_FORMAT_R8_UNORM:
			case VK_FORMAT_R8_SNORM:
			case VK_FORMAT_R8_USCALED:
			case VK_FORMAT_R8_SSCALED:
			case VK_FORMAT_R8_UINT:
			case VK_FORMAT_R8_SINT:
			case VK_FORMAT_R8_SRGB:
				return 1;

			case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
			case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
			case VK_FORMAT_R5G6B5_UNORM_PACK16:
			case VK_FORMAT_B5G6R5_UNORM_PACK16:
			case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
			case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
			case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SNORM:
			case VK_FORMAT_R8G8_USCALED:
			case VK_FORMAT_R8G8_SSCALED:
			case VK_FORMAT_R8G8_UINT:
			case VK_FORMAT_R8G8_SINT:
			case VK_FORMAT_R8G8_SRGB:
			case VK_FORMAT_R16_UNORM:
			case VK_FORMAT_R16_SNORM:
			case VK_FORMAT_R16_USCALED:
			case VK_FORMAT_R16_SSCALED:
			case VK_FORMAT_R16_UINT:
			case VK_FORMAT_R16_SINT:
			case VK_FORMAT_R16_SFLOAT:
				return 2;

			case VK_FORMAT_R8G8B8_UNORM:
			case VK_FORMAT_R8G8B8_SNORM:
			case VK_FORMAT_R8G8B8_USCALED:
			case VK_FORMAT_R8G8B8_SSCALED:
			case VK_FORMAT_R8G8B8_UINT:
			case VK_FORMAT_R8G8B8_SINT:
			case VK_FORMAT_R8G8B8_SRGB:
			case VK_FORMAT_B8G8R8_UNORM:
			case VK_FORMAT_B8G8R8_SNORM:
			case VK_FORMAT_B8G8R8_USCALED:
			case VK_FORMAT_B8G8R8_SSCALED:
			case VK_FORMAT_B8G8R8_UINT:
			case VK_FORMAT_B8G8R8_SINT:
			case VK_FORMAT_B8G8R8_SRGB:
				return 3;

			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SNORM:
			case VK_FORMAT_R8G8B8A8_USCALED:
			case VK_FORMAT_R8G8B8A8_SSCALED:
			case VK_FORMAT_R8G8B8A8_UINT:
			case VK_FORMAT_R8G8B8A8_SINT:
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SNORM:
			case VK_FORMAT_B8G8R8A8_USCALED:
			case VK_FORMAT_B8G8R8A8_SSCALED:
			case VK_FORMAT_B8G8R8A8_UINT:
			case VK_FORMAT_B8G8R8A8_SINT:
			case VK_FORMAT_B8G8R8A8_SRGB:
			case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
			case VK_FORMAT_A8B8G8R8_UINT_PACK32:
			case VK_FORMAT_A8B8G8R8_SINT_PACK32:
			case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
			case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_UINT_PACK32:
			case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			case VK_FORMAT_A2B10G10R10_SINT_PACK32:
			case VK_FORMAT_R16G16_UNORM:
			case VK_FORMAT_R16G16_SNORM:
			case VK_FORMAT_R16G16_USCALED:
			case VK_FORMAT_R16G16_SSCALED:
			case VK_FORMAT_R16G16_UINT:
			case VK_FORMAT_R16G16_SINT:
			case VK_FORMAT_R16G16_SFLOAT:
			case VK_FORMAT_R32_UINT:
			case VK_FORMAT_R32_SINT:
			case VK_FORMAT_R32_SFLOAT:
				return 4;

			case VK_FORMAT_R16G16B16_UNORM:
			case VK_FORMAT_R16G16B16_SNORM:
			case VK_FORMAT_R16G16B16_USCALED:
			case VK_FORMAT_R16G16B16_SSCALED:
			case VK_FORMAT_R16G16B16_UINT:
			case VK_FORMAT_R16G16B16_SINT:
			case VK_FORMAT_R16G16B16_SFLOAT:
				return 6;

			case VK_FORMAT_R16G16B16A16_UNORM:
			case VK_FORMAT_R16G16B16A16_SNORM:
			case VK_FORMAT_R16G16B16A16_USCALED:
			case VK_FORMAT_R16G16B16A16_SSCALED:
			case VK_FORMAT_R16G16B16A16_UINT:
			case VK_FORMAT_R16G16B16A16_SINT:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R32G32_UINT:
			case VK_FORMAT_R32G32_SINT:
			case VK_FORMAT_R32G32_SFLOAT:
				return 8;

			case VK_FORMAT_R32G32B32_UINT:
			case VK_FORMAT_R32G32B32_SINT:
			case VK_FORMAT_R32G32B32_SFLOAT:
				return 12;

			case VK_FORMAT_R32G32B32A32_UINT:
			case VK_FORMAT_R32G32B32A32_SINT:
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return 16;

			default:
				SY_ASSERT(false, "Unsupported format.");
				return 0;
			}
		}

		constexpr static size_t ToNumberOfComponents(const VkFormat format)
		{
			switch (format)
			{
			case VK_FORMAT_R8_UNORM:
			case VK_FORMAT_R8_SNORM:
			case VK_FORMAT_R8_USCALED:
			case VK_FORMAT_R8_SSCALED:
			case VK_FORMAT_R8_UINT:
			case VK_FORMAT_R8_SINT:
			case VK_FORMAT_R8_SRGB:
			case VK_FORMAT_R16_UNORM:
			case VK_FORMAT_R16_SNORM:
			case VK_FORMAT_R16_USCALED:
			case VK_FORMAT_R16_SSCALED:
			case VK_FORMAT_R16_UINT:
			case VK_FORMAT_R16_SINT:
			case VK_FORMAT_R16_SFLOAT:
			case VK_FORMAT_R32_UINT:
			case VK_FORMAT_R32_SINT:
			case VK_FORMAT_R32_SFLOAT:
				return 1;

			case VK_FORMAT_R4G4_UNORM_PACK8:
			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SNORM:
			case VK_FORMAT_R8G8_USCALED:
			case VK_FORMAT_R8G8_SSCALED:
			case VK_FORMAT_R8G8_UINT:
			case VK_FORMAT_R8G8_SINT:
			case VK_FORMAT_R8G8_SRGB:
			case VK_FORMAT_R16G16_UNORM:
			case VK_FORMAT_R16G16_SNORM:
			case VK_FORMAT_R16G16_USCALED:
			case VK_FORMAT_R16G16_SSCALED:
			case VK_FORMAT_R16G16_UINT:
			case VK_FORMAT_R16G16_SINT:
			case VK_FORMAT_R16G16_SFLOAT:
			case VK_FORMAT_R32G32_UINT:
			case VK_FORMAT_R32G32_SINT:
			case VK_FORMAT_R32G32_SFLOAT:
				return 2;

			case VK_FORMAT_R5G6B5_UNORM_PACK16:
			case VK_FORMAT_B5G6R5_UNORM_PACK16:
			case VK_FORMAT_R8G8B8_UNORM:
			case VK_FORMAT_R8G8B8_SNORM:
			case VK_FORMAT_R8G8B8_USCALED:
			case VK_FORMAT_R8G8B8_SSCALED:
			case VK_FORMAT_R8G8B8_UINT:
			case VK_FORMAT_R8G8B8_SINT:
			case VK_FORMAT_R8G8B8_SRGB:
			case VK_FORMAT_B8G8R8_UNORM:
			case VK_FORMAT_B8G8R8_SNORM:
			case VK_FORMAT_B8G8R8_USCALED:
			case VK_FORMAT_B8G8R8_SSCALED:
			case VK_FORMAT_B8G8R8_UINT:
			case VK_FORMAT_B8G8R8_SINT:
			case VK_FORMAT_B8G8R8_SRGB:
			case VK_FORMAT_R16G16B16_UNORM:
			case VK_FORMAT_R16G16B16_SNORM:
			case VK_FORMAT_R16G16B16_USCALED:
			case VK_FORMAT_R16G16B16_SSCALED:
			case VK_FORMAT_R16G16B16_UINT:
			case VK_FORMAT_R16G16B16_SINT:
			case VK_FORMAT_R16G16B16_SFLOAT:
			case VK_FORMAT_R32G32B32_UINT:
			case VK_FORMAT_R32G32B32_SINT:
			case VK_FORMAT_R32G32B32_SFLOAT:
				return 3;

			case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
			case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
			case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
			case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
			case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SNORM:
			case VK_FORMAT_R8G8B8A8_USCALED:
			case VK_FORMAT_R8G8B8A8_SSCALED:
			case VK_FORMAT_R8G8B8A8_UINT:
			case VK_FORMAT_R8G8B8A8_SINT:
			case VK_FORMAT_R8G8B8A8_SRGB:
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SNORM:
			case VK_FORMAT_B8G8R8A8_USCALED:
			case VK_FORMAT_B8G8R8A8_SSCALED:
			case VK_FORMAT_B8G8R8A8_UINT:
			case VK_FORMAT_B8G8R8A8_SINT:
			case VK_FORMAT_B8G8R8A8_SRGB:
			case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
			case VK_FORMAT_A8B8G8R8_UINT_PACK32:
			case VK_FORMAT_A8B8G8R8_SINT_PACK32:
			case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
			case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			case VK_FORMAT_A2R10G10B10_UINT_PACK32:
			case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			case VK_FORMAT_A2B10G10R10_SINT_PACK32:
			case VK_FORMAT_R16G16B16A16_UNORM:
			case VK_FORMAT_R16G16B16A16_SNORM:
			case VK_FORMAT_R16G16B16A16_USCALED:
			case VK_FORMAT_R16G16B16A16_SSCALED:
			case VK_FORMAT_R16G16B16A16_UINT:
			case VK_FORMAT_R16G16B16A16_SINT:
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R32G32B32A32_UINT:
			case VK_FORMAT_R32G32B32A32_SINT:
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return 4;

			default:
				SY_ASSERT(false, "Unsupported format.");
				return 0;
			}
		}

		constexpr static VkImageSubresourceRange ImageSubresourceRange(const VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, const uint32_t baseMipLevel = 0, const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0, const uint32_t layerCount = 1)
		{
			return{
				aspectMask,
				baseMipLevel,
				levelCount,
				baseArrayLayer,
				layerCount
			};
		}

		constexpr static VkImageMemoryBarrier ImageMemoryBarrier(const VkAccessFlags srcAccessMask, const VkAccessFlags dstAccessMask, const VkImageLayout oldLayout, const VkImageLayout newLayout, const VkImage image, const VkImageSubresourceRange imageSubresourceRange)
		{
			return
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = srcAccessMask,
				.dstAccessMask = dstAccessMask,
				.oldLayout = oldLayout,
				.newLayout = newLayout,
				.image = image,
				.subresourceRange = imageSubresourceRange
			};
		}

		VkRenderingAttachmentInfo DepthAttachmentInfo(const TextureView& depthStencil, float depthClearValue = 1.f, uint8_t stencilClearValue = 0);

		constexpr static bool IsDepthStencilFormat(const VkFormat format)
		{
			return (format >= VK_FORMAT_D16_UNORM) && (format <= VK_FORMAT_D32_SFLOAT_S8_UINT);
		}

		/** Barrier */
		enum class EAccessPattern
		{
			None,

			CommandBufferRead,
			IndirectBuffer,
			IndexBuffer,
			VertexBuffer,

			VertexShaderReadUniformBuffer,
			VertexShaderReadSampledImage,
			VertexShaderReadUniformTexelBuffer,
			VertexShaderReadGeneral,

			TessellationControlShaderReadUniformBuffer,
			TessellationControlShaderReadSampledImage,
			TessellationControlShaderReadUniformTexelBuffer,
			TessellationControlShaderReadGeneral,

			TessellationEvaluationShaderReadUniformBuffer,
			TessellationEvaluationShaderReadSampledImage,
			TessellationEvaluationShaderReadUniformTexelBuffer,
			TessellationEvaluationShaderReadGeneral,

			GeometryShaderReadUniformBuffer,
			GeometryShaderReadSampledImage,
			GeometryShaderReadUniformTexelBuffer,
			GeometryShaderReadGeneral,

			TaskShaderReadUniformBuffer,
			TaskShaderReadSampledImage,
			TaskShaderReadUniformTexelBuffer,
			TaskShaderReadGeneral,

			MeshShaderReadUniformBuffer,
			MeshShaderReadSampledImage,
			MeshShaderReadUniformTexelBuffer,
			MeshShaderReadGeneral,

			TransformFeedbackCounterRead,
			FragmentDensityMapRead,
			ShadingRateRead,

			FragmentShaderReadUniformBuffer,
			FragmentShaderReadSampledImage,
			FragmentShaderReadUniformTexelBuffer,
			FragmentShaderReadColorInputAttachment,
			FragmentShaderReadDepthStencilInputAttachment,
			FragmentShaderReadGeneral,

			ColorAttachmentRead,
			ColorAttachmentAdvancedBlending,
			DepthStencilAttachmentRead,

			ComputeShaderReadUniformBuffer,
			ComputeShaderReadSampledImage,
			ComputeShaderReadUniformTexelBuffer,
			ComputeShaderReadGeneral,

			AnyShaderReadUniformBuffer,
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
			VkPipelineStageFlags PipelineStage;
			VkAccessFlags Access;
			VkImageLayout ImageLayout;
		};

		static AccessPattern QueryAccessPattern(const EAccessPattern pattern)
		{
			switch (pattern)
			{
			case EAccessPattern::None:
				return { 0, 0, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::CommandBufferRead:
				return { VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV, VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::IndirectBuffer:
				return { VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::IndexBuffer:
				return { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::VertexBuffer:
				return { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::VertexShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::VertexShaderReadSampledImage:
			case EAccessPattern::VertexShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::VertexShaderReadGeneral:
				return { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TessellationControlShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::TessellationControlShaderReadSampledImage:
			case EAccessPattern::TessellationControlShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::TessellationControlShaderReadGeneral:
				return { VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TessellationEvaluationShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::TessellationEvaluationShaderReadSampledImage:
			case EAccessPattern::TessellationEvaluationShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::TessellationEvaluationShaderReadGeneral:
				return { VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::GeometryShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::GeometryShaderReadSampledImage:
			case EAccessPattern::GeometryShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::GeometryShaderReadGeneral:
				return { VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TaskShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::TaskShaderReadSampledImage:
			case EAccessPattern::TaskShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::TaskShaderReadGeneral:
				return { VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::MeshShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::MeshShaderReadSampledImage:
			case EAccessPattern::MeshShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::MeshShaderReadGeneral:
				return { VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TransformFeedbackCounterRead:
				return { VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::FragmentDensityMapRead:
				return { VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT, VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT };

			case EAccessPattern::ShadingRateRead:
				return { VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV };

			case EAccessPattern::FragmentShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::FragmentShaderReadSampledImage:
			case EAccessPattern::FragmentShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::FragmentShaderReadColorInputAttachment:
				return { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::FragmentShaderReadDepthStencilInputAttachment:
				return { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };

			case EAccessPattern::FragmentShaderReadGeneral:
				return { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::ColorAttachmentRead:
				return { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			case EAccessPattern::ColorAttachmentAdvancedBlending:
				return { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			case EAccessPattern::DepthStencilAttachmentRead:
				return { VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };

			case EAccessPattern::ComputeShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::ComputeShaderReadSampledImage:
			case EAccessPattern::ComputeShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::ComputeShaderReadGeneral:
				return { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::AnyShaderReadUniformBuffer:
				return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::AnyShaderReadSampledImage:
			case EAccessPattern::AnyShaderReadUniformTexelBuffer:
				return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

			case EAccessPattern::AnyShaderReadGeneral:
				return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TransferRead:
				return { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL };

			case EAccessPattern::HostRead:
				return { VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_READ_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::Present:
				return { 0, 0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };

			case EAccessPattern::ConditionalRenderingRead:
				return { VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT, VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::RayTracingShaderAccelerationStructureRead:
				return { VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::AccelerationStructureBuildRead:
				return { VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::EndOfRead:
				return { 0, 0, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::CommandBufferWrite:
				return { VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV, VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::VertexShaderWrite:
				return { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TessellationControlShaderWrite:
				return { VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TessellationEvaluationShaderWrite:
				return { VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::GeometryShaderWrite:
				return { VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TaskShaderWrite:
				return { VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::MeshShaderWrite:
				return { VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TransformFeedbackWrite:
				return { VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::TransformFeedbackCounterWrite:
				return { VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::FragmentShaderWrite:
				return { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::ColorAttachmentWrite:
				return { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			case EAccessPattern::DepthStencilAttachmentWrite:
				return { VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

			case EAccessPattern::DepthAttachmentWriteStencilReadOnly:
				return { VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR };

			case EAccessPattern::StencilAttachmentWriteDepthReadOnly:
				return { VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR };

			case EAccessPattern::ComputeShaderWrite:
				return { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::AnyShaderWrite:
				return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::TransferWrite:
				return { VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL };

			case EAccessPattern::HostPreInitialized:
				return { VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED };

			case EAccessPattern::HostWrite:
				return { VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };

			case EAccessPattern::AccelerationStructureBuildWrite:
				return { VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR, VK_IMAGE_LAYOUT_UNDEFINED };

			case EAccessPattern::ColorAttachmentReadWrite:
				return { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			case EAccessPattern::General:
			default:
				return { VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL };
			}
		}
	}
}