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

	constexpr size_t NumMaxInFlightFrames = 2;
}