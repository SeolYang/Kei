#pragma once
#pragma warning(disable : 4819)
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

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <volk.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
#include <spirv_reflect.h>

#include <robin_hood.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_mixer.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <entt/entt.hpp>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <stb_image.h>

#include <ktx.h>
using KTXTexture2UniquePtr = std::unique_ptr<ktxTexture2, std::function<void(ktxTexture2*)>>;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <stduuid/uuid.h>

#include <magic_enum.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/version.h>

#include <meshoptimizer.h>

#include <Core/Types.h>
#include <Core/Subsystem.h>
#include <Core/Extent.h>
#include <Core/Assert.h>
#include <Core/Pool.hpp>
#include <Core/Timer.h>
#include <Core/Utils.h>
#include <Core/HandleManager.h>

#include <VK/VulkanEnums.h>
#include <VK/VulkanStructures.h>
#include <VK/VulkanUtils.h>
#include <VK/VulkanConstants.h>
#include <VK/Synchronization.h>

#include <Math/MathUtils.h>

#include <Render/Vertex.h>
#include <Render/Mesh.h>
