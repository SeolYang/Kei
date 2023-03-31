#pragma once
#include <PCH.h>
#include <VK/VertexInputBuilder.h>

namespace sy::render
{
using IndexType = uint32_t;

enum class EVertexAttributeType
{
    Color,
    Position,
    TexCoords0,
    Normal,
    Bitangent,
    Tangent
};

struct VertexPT0
{
    glm::vec3 Position;
    glm::vec2 TexCoords0;
};

struct VertexPT0N
{
    glm::vec3 Position;
    glm::vec2 TexCoords0;
    glm::vec3 Normal;
};

/**
* P => Position <vec3>
* T(N) => TexCoords <vec2>
* N => Normal <vec3>
* T => Tangent <vec3>,
* B => Bi-tangent <vec3>
* W(N) = Weights <float>
*/
enum class EVertexType
{
    PT0,
    PT0N,
};

constexpr size_t SizeOfVertex(const EVertexType type)
{
    switch (type)
    {
        case EVertexType::PT0:
            return sizeof(VertexPT0);

        case EVertexType::PT0N:
            return sizeof(VertexPT0N);
    }

    return 0;
}

template <typename VertexType>
vk::VertexInputBuilder BuildVertexInputLayout();

template <>
inline vk::VertexInputBuilder BuildVertexInputLayout<VertexPT0>()
{
    using Vertex       = VertexPT0;
    const auto builder = vk::VertexInputBuilder{}
                             .AddVertexInputBinding<Vertex>(0, VK_VERTEX_INPUT_RATE_VERTEX)
                             .AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position))
                             .AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoords0));
    return builder;
}

template <>
inline vk::VertexInputBuilder BuildVertexInputLayout<VertexPT0N>()
{
    using Vertex = VertexPT0N;
    vk::VertexInputBuilder builder;
    builder.AddVertexInputBinding<Vertex>(0, VK_VERTEX_INPUT_RATE_VERTEX)
        .AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position))
        .AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoords0))
        .AddVertexInputAttribute(2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Normal));
    return builder;
}

inline std::optional<vk::VertexInputBuilder> BuildVertexInputLayout(const EVertexType type)
{
    switch (type)
    {
        case EVertexType::PT0:
            return BuildVertexInputLayout<VertexPT0>();

        case EVertexType::PT0N:
            return BuildVertexInputLayout<VertexPT0N>();
    }

    return std::nullopt;
}

template <typename VertexType>
std::optional<Range<size_t>> QueryRangeOfVertexAttribute(const EVertexAttributeType type);

template <>
inline std::optional<Range<size_t>> QueryRangeOfVertexAttribute<VertexPT0>(const EVertexAttributeType attribute)
{
    switch (attribute)
    {
        case EVertexAttributeType::Position:
            return Range<size_t>{.Offset = offsetof(VertexPT0, Position), .Size = sizeof(VertexPT0::Position)};

        case EVertexAttributeType::TexCoords0:
            return Range<size_t>{.Offset = offsetof(VertexPT0, TexCoords0), .Size = sizeof(VertexPT0::TexCoords0)};

        default:
            return std::nullopt;
    }

    return std::nullopt;
}

template <>
inline std::optional<Range<size_t>> QueryRangeOfVertexAttribute<VertexPT0N>(const EVertexAttributeType attribute)
{
    switch (attribute)
    {
        case EVertexAttributeType::Position:
            return Range<size_t>{.Offset = offsetof(VertexPT0N, Position), .Size = sizeof(VertexPT0::Position)};

        case EVertexAttributeType::TexCoords0:
            return Range<size_t>{.Offset = offsetof(VertexPT0N, TexCoords0), .Size = sizeof(VertexPT0::TexCoords0)};

        case EVertexAttributeType::Normal:
            return Range<size_t>{.Offset = offsetof(VertexPT0N, Normal), .Size = sizeof(VertexPT0N::Normal)};

        default:
            return std::nullopt;
    }

    return std::nullopt;
}

inline std::optional<Range<size_t>> QueryRangeOfVertexAttribute(const EVertexType vertex, const EVertexAttributeType attribute)
{
    switch (vertex)
    {
        case EVertexType::PT0:
            return QueryRangeOfVertexAttribute<VertexPT0>(attribute);
        case EVertexType::PT0N:
            return QueryRangeOfVertexAttribute<VertexPT0N>(attribute);
        default:
            return std::nullopt;
    }

    return std::nullopt;
}

} // namespace sy::render
