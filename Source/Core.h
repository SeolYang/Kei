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
#include <tuple>
#include <fstream>
#include <concurrent_queue.h>

namespace sy
{
	namespace chrono = std::chrono;
	namespace fs = std::filesystem;

	template <typename T>
	using Ref = std::reference_wrapper<T>;
	template <typename T>
	using CRef = Ref<const T>;
	template <typename T>
	using RefVec = std::vector<Ref<T>>;
	template <typename T>
	using CRefVec = std::vector<CRef<T>>;
	template <typename T>
	using RefSpan = std::span<Ref<T>>;
	template <typename T>
	using CRefSpan = std::span<CRef<T>>;

	using RWLock = std::unique_lock<std::shared_mutex>;
	using ReadLock = std::shared_lock<std::shared_mutex>;
}

/** Vulkan Headers */
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <volk.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

/** robinhood hash */
#include <robin_hood.h>

/** SDL2 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_mixer.h>

/** spdlog */
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

/** entt */
#include <entt/entt.hpp>

/** taskflow */
#include <taskflow/taskflow.hpp>

/** glm */
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

/** stbi */
#include <stb_image.h>

/** lz4 compression */
#include <lz4.h>

/** nlohmann-json */
#include <nlohmann/json.hpp>

/** Internals */
#include <UniqueHandle.h>
#include <NamedType.h>
#include <Extent.h>
#include <TellusAssert.h>
#include <Pool.hpp>

#include <VK/VulkanTypes.h>
#include <VK/VulkanWrapper.hpp>

#include <Timer.h>
#include <Utils.h>
#include <MathUtils.h>