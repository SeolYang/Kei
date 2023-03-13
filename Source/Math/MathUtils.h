#pragma once
#include <PCH.h>

namespace sy::math
{
	static auto PerspectiveYFlipped(const float fovy, const float aspectRatio, const float nearPlane,
	                                const float farPlane)
	{
		auto perspective = glm::perspective(fovy, aspectRatio, nearPlane, farPlane);
		perspective[ 1 ][ 1 ] *= -1.f;
		return perspective;
	}

	constexpr bool IsPowOfTwo(const size_t value)
	{
		return (value != 0) && ((value & (value - 1)) == 0);
	}

	constexpr bool IsPowOfTwo(const Extent3D<uint32_t> extent)
	{
		return IsPowOfTwo(extent.width) && (extent.width == extent.height) && (extent.height == extent.depth);
	}
}
