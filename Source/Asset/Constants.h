#pragma once
#include <string_view>

namespace sy::asset::constants::ext
{
constexpr std::string_view Asset = "asset";
constexpr std::string_view Blob  = "blob";
constexpr std::string_view JPG   = "jpg";
constexpr std::string_view JPEG  = "jpeg";
constexpr std::string_view PNG   = "png";
constexpr std::string_view OBJ   = "obj";
constexpr std::string_view GLTF  = "gltf";
constexpr std::string_view FBX   = "fbx";
} // namespace sy::asset::constants::ext

namespace sy::asset::constants::path
{
constexpr std::string_view AssetRootRelative  = "Assets";
constexpr std::string_view AssetImportConfigs = "Assets/AssetImportConfigs.meta";
} // namespace sy::asset::constants::path

namespace sy::asset::constants::metadata::key
{
constexpr std::string_view Name                       = "Name";
constexpr std::string_view TypeHash                   = "TypeHash";
constexpr std::string_view CompressionMode            = "CompressionMode";
constexpr std::string_view CompressionQuality         = "CompressionQuality";
constexpr std::string_view CompressionFlag            = "IsCompressed";
constexpr std::string_view Format                     = "Format";
constexpr std::string_view Extent                     = "Extent";
constexpr std::string_view Offset                     = "Offset";
constexpr std::string_view Size                       = "Size";
constexpr std::string_view MaterialAsset              = "MaterialAsset";
constexpr std::string_view Sampler                    = "Sampler";
constexpr std::string_view VertexType                 = "VertexType";
constexpr std::string_view MeshDataList               = "Meshes";
constexpr std::string_view BaseTexture                = "BaseTexture";
constexpr std::string_view VerticesBlobSize           = "VerticesBlobSize";
constexpr std::string_view IndicesBlobSize            = "IndicesBlobSize";
constexpr std::string_view VerticesBlobRange          = "VerticesBlobRange";
constexpr std::string_view IndicesBlobRange           = "IndicesBlobRange";
constexpr std::string_view NumVertices                = "NumVertices";
constexpr std::string_view NumIndices                 = "NumIndices";
constexpr std::string_view Quality                    = "Quality";
constexpr std::string_view GenMips                    = "GenerateMips";
constexpr std::string_view GenerateMaterialPerMesh    = "GenerateMaterialPerMesh";
constexpr std::string_view Compression                = "Compression";
constexpr std::string_view ConvertToLeftHanded        = "ConvertToLeftHanded";
constexpr std::string_view FlipWindingOrder           = "FlipWindingOrder";
constexpr std::string_view FlipTextureCoordinates     = "FlipTexCoords";
constexpr std::string_view MakedLeftHanded            = "MakeLeftHanded";
constexpr std::string_view GenerateTextureCoordinates = "GenerateTexCoords";
constexpr std::string_view GenerateNormals            = "GenrateNormals";
constexpr std::string_view GenerateSmoothNormals      = "GenerateSmoothNormals";
constexpr std::string_view CalculateTangentSpace      = "CalculateTangentSpace";
constexpr std::string_view PretransformVertices       = "PretransformVertices";
constexpr std::string_view GenerateMipsWhenImport     = "GenerateMipsWhenImport";
constexpr std::string_view Configuration              = "Configuration";
constexpr std::string_view RequireReimportAsset       = "RequiredReimportAsset";
} // namespace sy::asset::constants::metadata::key
