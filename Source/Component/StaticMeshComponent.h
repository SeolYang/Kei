#pragma once
#include <PCH.h>

namespace sy::render
{
class Mesh;
class Material;
} // namespace sy::render

namespace sy::component
{
struct StaticMeshComponent
{
    Handle<render::Mesh>     Mesh;
    Handle<render::Material> Material;
};
} // namespace sy::component
