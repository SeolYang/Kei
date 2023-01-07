#pragma once
#include <Core/Core.h>

namespace sy
{
	template<typename T>
	struct Handle
	{
		size_t Value = InvalidValue;
		constexpr static size_t InvalidValue = -1;
	};
}