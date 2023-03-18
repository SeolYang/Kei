#pragma once
#include <PCH.h>

namespace sy
{
class Subsystem : public NonCopyable
{
public:
    virtual ~Subsystem() = default;

    virtual void Startup()  = 0;
    virtual void Shutdown() = 0;
};
} // namespace sy
