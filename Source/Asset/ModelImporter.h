#pragma once
#include <PCH.h>

namespace sy::asset
{
struct ModelImportConfig
{
    ModelImportConfig& SetVertexType(const render::EVertexType type)
    {
        vertexType = type;
        return *this;
    }

    ModelImportConfig& SetGenerateMaterialPerMesh(const bool enabled)
    {
        bGenMaterialPerMesh = enabled;
        return *this;
    }

    ModelImportConfig& SetEnableCompression(const bool enabled)
    {
        bEnableCompression = enabled;
        return *this;
    }

    ModelImportConfig& SetConvertToLeftHanded(const bool enabled)
    {
        bConvertToLeftHanded = enabled;
        return *this;
    }

    ModelImportConfig& SetFlipWindingOrder(const bool enabled)
    {
        bFlipWingdingOrder = enabled;
        return *this;
    }

    ModelImportConfig& SetFlipTextureCoordinates(const bool enabled)
    {
        bFlipUVs = enabled;
        return *this;
    }

    ModelImportConfig& SetMakeLeftHanded(const bool enabled)
    {
        bMakeLeftHanded = enabled;
        return *this;
    }

    ModelImportConfig& SetGenerateTextureCoordiantes(const bool enabled)
    {
        bGenUVCoords = enabled;
        return *this;
    }

    ModelImportConfig& SetGenerateNormals(const bool enabled)
    {
        bGenNormals = enabled;
        return *this;
    }

    ModelImportConfig& SetGenerateSmoothNormals(const bool enabled)
    {
        bGenSmoothNormals = enabled;
        return *this;
    }

    ModelImportConfig& SetCalculateTangentSpace(const bool enabled)
    {
        bCalcTagentSpace = enabled;
        return *this;
    }

    ModelImportConfig& SetPretransformVertices(const bool enabled)
    {
        bPretransformVertices = enabled;
        return *this;
    }

    [[nodiscard]] auto GetVertexType() const { return vertexType; }
    [[nodiscard]] bool IsGenerateMaterialPerMesh() const { return bGenMaterialPerMesh; }
    [[nodiscard]] bool IsCompressionEnabled() const { return bEnableCompression; }
    [[nodiscard]] bool IsConvertToLeftHanded() const { return bConvertToLeftHanded; }
    [[nodiscard]] bool IsFlipWindingOrder() const { return bFlipWingdingOrder; }
    [[nodiscard]] bool IsFlipTextureCoordinates() const { return bFlipUVs; }
    [[nodiscard]] bool IsMakeLeftHanded() const { return bMakeLeftHanded; }
    [[nodiscard]] bool IsGenerateTextureCoordinates() const { return bGenUVCoords; }
    [[nodiscard]] bool IsGenerateNormals() const { return bGenNormals; }
    [[nodiscard]] bool IsGenerateSmoothNormals() const { return bGenSmoothNormals; }
    [[nodiscard]] bool IsCalculateTangentSpace() const { return bCalcTagentSpace; }
    [[nodiscard]] bool IsPretransformVertices() const { return bPretransformVertices; }

private:
    render::EVertexType vertexType           = render::EVertexType::PT0N;
    bool                bGenMaterialPerMesh  = false;
    bool                bEnableCompression   = true;
    bool                bConvertToLeftHanded = false;
    bool                bFlipWingdingOrder   = false;
    bool                bFlipUVs             = false;
    bool                bMakeLeftHanded      = false;
    bool                bGenUVCoords         = false;
    bool                bGenNormals          = false;
    bool                bGenSmoothNormals    = false;
    bool                bCalcTagentSpace     = false;
    /** #warn	This flag will be remove animations. Use this for only test purpose as possible. */
    bool bPretransformVertices = false;

    /*** RESERVED FLAGS TO IMPLEMENT ***/
    // #todo	Implement it!
    // #warn	Not implemented feature.
    bool bIncludeAnimations = false;

    /**
	* #todo	Implement feature!
	* #warn	Not implemented feature.
	* #impl_ideas
	* Save hierarchy info to metadata.
	* (Hierarchy relationship(using name of mesh), Transformation)
	* It makes able to generate set of Entities, which are include those informations from Asset.
	*/
    bool bIncludeHierarchyInformations = false;
};

class ModelImporter
{
public:
    static bool Import(const fs::path& path, ModelImportConfig config);

private:
    ModelImporter()  = default;
    ~ModelImporter() = default;
};
} // namespace sy::asset