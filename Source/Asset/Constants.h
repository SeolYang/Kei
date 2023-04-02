#pragma once
#include <string_view>

namespace sy::asset::constants::fs::ext
{
constexpr std::string_view Asset = "asset";
constexpr std::string_view Blob  = "blob";
} // namespace sy::asset::constants::fs::ext

namespace sy::asset::constants::metadata::key
{
constexpr std::string_view Name               = "Name";
constexpr std::string_view TypeHash           = "TypeHash";
constexpr std::string_view CompressionMode    = "CompressionMode";
constexpr std::string_view CompressionQuality = "CompressionQuality";
constexpr std::string_view CompressionFlag    = "IsCompressed";
constexpr std::string_view Format             = "Format";
constexpr std::string_view Extent             = "Extent";
constexpr std::string_view Offset             = "Offset";
constexpr std::string_view Size               = "Size";
constexpr std::string_view MaterialAsset      = "MaterialAsset";
constexpr std::string_view Sampler            = "Sampler";
constexpr std::string_view VertexType         = "VertexType";
constexpr std::string_view MeshDataList       = "Meshes";
constexpr std::string_view BaseTexture        = "BaseTexture";
constexpr std::string_view VerticesBlobSize   = "VerticesBlobSize";
constexpr std::string_view IndicesBlobSize    = "IndicesBlobSize";
constexpr std::string_view VerticesBlobRange  = "VerticesBlobRange";
constexpr std::string_view IndicesBlobRange   = "IndicesBlobRange";
constexpr std::string_view NumVertices        = "NumVertices";
constexpr std::string_view NumIndices         = "NumIndices";
constexpr std::string_view Quality            = "Quality";
} // namespace sy::asset::constants::metadata::key
