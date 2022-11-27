#pragma once
#include <Core.h>

namespace sy
{
	template <typename T>
	class UniqueHandle
	{
	public:
		UniqueHandle(const T handle, const std::function<void(T)>& customDeleter) :
			handle(handle),
			customDeleter(customDeleter)
		{
		}

		~UniqueHandle()
		{
			customDeleter(handle);
		}

		[[nodiscard]] T GetHandle() const { return handle; }
	private:
		T handle;
		const std::function<void(T)> customDeleter;

	};
}