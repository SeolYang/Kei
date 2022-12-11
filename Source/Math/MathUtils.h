#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace math
	{
		static auto PerspectiveYFlipped(const float fovy, const float aspectRatio, const float nearPlane, const float farPlane)
		{
			auto perspective = glm::perspective(fovy, aspectRatio, nearPlane, farPlane);
			perspective[1][1] *= -1.f;
			return perspective;
		}
	}
}
