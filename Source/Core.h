#pragma once

/** Standard Library */
#include <vector>
#include <array>
#include <queue>
#include <unordered_map>
#include <map>
#include <memory>
#include <sstream>
#include <string_view>
#include <string>
#include <algorithm>
#include <atomic>
#include <optional>
#include <variant>
#include <type_traits>
#include <random>
#include <chrono>
#include <numeric>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <thread>
#include <utility>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <ranges>
#include <numbers>

namespace sy
{
	namespace chrono = std::chrono;
	namespace fs = std::filesystem;
}

/** Vulkan Headers */
#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <vk_mem_alloc.h>

#include <volk.h>
#include <VkBootstrap.h>

/** SDL2 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

/** spdlog */
#include <spdlog/spdlog.h>

#include <NamedType.h>
#include <Extent.h>
#include <TellusAssert.h>
#include <VulkanTypes.h>

#include <ECS.h>
#include <Timer.h>