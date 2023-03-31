#pragma once
#include <PCH.h>

namespace sy
{
class Timer final : public Subsystem
{
public:
    void Startup() override
    {
        spdlog::info("Startup Timer.");
        Begin();
    }

    void Shutdown() override
    {
        /** Empty */
        spdlog::info("Shutdown Timer.");
    }

    void Begin()
    {
        begin = chrono::high_resolution_clock::now();
    }

    void End()
    {
        deltaTime = (chrono::high_resolution_clock::now() - begin);
    }

    [[nodiscard]] uint64_t GetDeltaTimeNanos() const
    {
        return deltaTime.count();
    }

    [[nodiscard]] uint64_t GetDeltaTimeMillis() const
    {
        return chrono::duration_cast<chrono::milliseconds>(deltaTime).count();
    }

    [[nodiscard]] double GetDeltaTimeF64() const
    {
        return GetDeltaTimeNanos() * 1e-09;
    }

    [[nodiscard]] float GetDeltaTime() const
    {
        return static_cast<float>(GetDeltaTimeF64());
    }

private:
    chrono::steady_clock::time_point begin     = chrono::high_resolution_clock::now();
    chrono::nanoseconds              deltaTime = chrono::nanoseconds(0);
};
} // namespace sy
