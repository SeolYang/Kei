#pragma once
#include <PCH.h>

namespace sy
{
	template<typename T>
	struct Handle
	{
	public:
		size_t Value = InvalidValue;

		[[nodiscard]] operator bool() const { return Value != InvalidValue; }

	private:
		constexpr static size_t InvalidValue = std::numeric_limits<size_t>::max();
	};
}