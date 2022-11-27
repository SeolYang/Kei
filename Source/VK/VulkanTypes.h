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

	constexpr size_t NumMaxInFlightFrames = 2;
}