#pragma once
#include <PCH.h>

namespace sy
{
	class Timer final : public NonCopyable
	{
	public:
		void Startup() noexcept
		{
			Begin();
		}

		void Begin() noexcept
		{
			begin = chrono::high_resolution_clock::now();
		}

		void End() noexcept
		{
			deltaTime = (chrono::high_resolution_clock::now() - begin);
		}

		[[nodiscard]] uint64_t GetDeltaTimeNanos() const noexcept
		{
			return deltaTime.count();
		}

		[[nodiscard]] uint64_t GetDeltaTimeMillis() const noexcept
		{
			return chrono::duration_cast<chrono::milliseconds>(deltaTime).count();
		}

		[[nodiscard]] double GetDeltaTimeF64() const noexcept
		{
			return GetDeltaTimeNanos() * 1e-09;
		}

		[[nodiscard]] float GetDeltaTime() const noexcept
		{
			return static_cast<float>(GetDeltaTimeF64());
		}

	private:
		chrono::steady_clock::time_point begin = chrono::high_resolution_clock::now();
		chrono::nanoseconds deltaTime = chrono::nanoseconds(0);
	};
} // namespace sy
