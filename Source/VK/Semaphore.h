#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::vk
{
class Semaphore : public VulkanWrapper<VkSemaphore>
{
public:
    Semaphore(std::string_view name, VulkanContext& vulkanContext, bool bIsBinarySemaphore = false);
    ~Semaphore() = default;

	void IncrementValue() { ++value; }
    [[nodiscard]] uint64_t GetCurrentValue() const { return value; }
    [[nodiscard]] bool IsBinarySemaphore() const { return bIsBinarySemaphore; }

	void Wait(uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;
    void Signal() const;

private:
    const bool bIsBinarySemaphore;
    uint64_t value = 0;
};
} // namespace sy::vk
