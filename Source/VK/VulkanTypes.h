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

	static VkPipelineBindPoint PipelineTypeToBindPoint(const EPipelineType type)
	{
		switch (type)
		{
		default:
		case EPipelineType::Graphics:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case EPipelineType::Compute:
			return VK_PIPELINE_BIND_POINT_COMPUTE;
		case EPipelineType::RayTracing:
			return VK_PIPELINE_BIND_POINT_RAY_TRACING_NV;
		}
	}

	constexpr uint32_t MaxBindlessResourcesPerDescriptor = 2048;
	static uint32_t QueryBindlessBindingOfDescriptor(const VkDescriptorType type)
	{
		switch (type)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
			return 0;
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			return 1;
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			return 2;
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			return 3;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			return 4;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			return 5;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			return 6;
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			return 7;
		default:
			SY_ASSERT(false, "Unsupported Bindless descriptor type.");
			return std::numeric_limits<uint32_t>::max();
		}
	}

	constexpr size_t NumMaxInFlightFrames = 2;
}