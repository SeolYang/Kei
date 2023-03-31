#pragma once
#include <PCH.h>

namespace sy::asset
{
struct ModelImportConfig
{
    const bool          bGenMaterialPerMesh  = false;
    render::EVertexType VertexType           = render::EVertexType::PT0N;
    const bool          bEnableCompression   = true;
    const bool          bEnableOptimization  = false;
    const bool          bConvertToLeftHanded = false;
    const bool          bFlipWingdingOrder   = false;
    const bool          bFlipUVs             = false;
    const bool          bMakeLeftHanded      = false;
    const bool          bGenUVCoords         = false;
    const bool          bGenNormals          = false;
    const bool          bGenSmoothNormals    = false;
    const bool          bCalcTagentSpace     = false;
    /** #warn	This flag will be remove animations. Use this for only test purpose as possible. */
    const bool bPretransformVertices = false;

    /*** RESERVED FLAGS TO IMPLEMENT ***/
    // #todo	Implement it!
    // #warn	Not implemented feature.
    const bool bIncludeAnimations = false;

	/**
	* #todo	Implement feature!
	* #warn	Not implemented feature.
	* #impl_ideas
	* Save hierarchy info to metadata.
	* (Hierarchy relationship(using name of mesh), Transformation)
	* It makes able to generate set of Entities, which are include those informations from Asset.
	*/
    const bool bIncludeHierarchyInformations = false;
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