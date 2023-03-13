#pragma once
#include <PCH.h>

namespace sy
{
	using HandleUnderType = size_t;

	template <typename T>
	struct Handle
	{
	public:
		HandleUnderType Value = InvalidValue;

		[[nodiscard]] bool IsValidHandleValue() const { return Value != InvalidValue;}

		[[nodiscard]] operator bool() const
		{
			return IsValidHandleValue();
		}

	private:
		constexpr static HandleUnderType InvalidValue = std::numeric_limits<HandleUnderType>::max();
	};
}
