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

	constexpr size_t NumMaxInFlightFrames = 2;
}