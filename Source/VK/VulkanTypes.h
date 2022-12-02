#pragma once
#include <Core.h>

namespace sy
{
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

	static VkPipelineBindPoint ToNative(const EPipelineType type)
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

	constexpr uint32_t MaxBindlessResourcesPerDescriptor = 2048;
	constexpr size_t NumMaxInFlightFrames = 2;
}