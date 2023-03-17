#pragma once
#pragma warning(disable:4819)
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

/** Vulkan Headers */
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <volk.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

/** Spirv-reflect */
#include <spirv_reflect.h>

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
#include <glm/gtx/quaternion.hpp>

/** stbi */
#include <stb_image.h>

/** lz4 compression */
#include <lz4.h>

/** nlohmann-json */
#include <nlohmann/json.hpp>

/** stduuid */
//#include <uuid.h>

/** magic-enum */
#include <magic_enum.hpp>

/** assimp */
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>

/** mesh optimizer */
#include <meshoptimizer.h>

/** Tiny gltf */
#include <tiny_gltf.h>

/** Internal - Core */
#include <Core/Types.h>
#include <Core/Handle.h>
#include <Core/Extent.h>
#include <Core/Assert.h>
#include <Core/Pool.hpp>
#include <Core/Timer.h>
#include <Core/ECS.h>
#include <Core/Utils.h>

/** Internal - Vulkan */
#include <VK/VulkanTypes.h>
#include <VK/VulkanWrapper.hpp>
#include <VK/Synchronization.h>

/** Internal - Math */
#include <Math/MathUtils.h>

/** Internal - Render */
#include <Render/Vertex.h>
#include <Render/Mesh.h>