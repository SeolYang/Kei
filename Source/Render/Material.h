#pragma once
#include <PCH.h>

namespace sy::vk
{
class Pipeline;
}

namespace sy::render
{
constexpr std::string_view DefaultMaterial = "DefaultMaterial";

class Material
{
public:
    Handle<vk::Descriptor> BaseTexture;
    // Handle<vk::Descriptor> SpecularTexture;
};
} // namespace sy::render
