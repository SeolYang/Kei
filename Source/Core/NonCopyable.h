#pragma once

namespace sy
{
class NonCopyable
{
public:
    virtual ~NonCopyable()                    = default;
    NonCopyable(const NonCopyable&)           = delete;
    NonCopyable& operator=(const NonCopyable) = delete;

protected:
    NonCopyable() = default;
};
} // namespace sy