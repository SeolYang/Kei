#pragma once
#include <Core/Core.h>

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

		UniqueHandle(const UniqueHandle&) = delete;
		UniqueHandle(UniqueHandle&& other) noexcept :
			handle(std::exchange(other.handle, T())),
			customDeleter(std::exchange(other.customDeleter, CustomDeleter_t()))
		{
		}

		UniqueHandle& operator=(const UniqueHandle&) = delete;
		UniqueHandle& operator=(UniqueHandle&& rhs)
		{
			handle = std::exchange(rhs.handle, T());
			customDeleter = std::exchange(rhs.customDeleter, CustomDeleter_t());
			return *this;
		}

		[[nodiscard]] T GetHandle() const { return handle; }

	private:
		using CustomDeleter_t = std::function<void(T)>;

		T handle;
		std::function<void(T)> customDeleter;

	};
}