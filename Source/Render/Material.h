#pragma once
#include <PCH.h>
#include <Render/Constants.h>

namespace sy::vk
{
class Pipeline;
}

namespace sy::render
{
class Material
{
public:
    Handle<vk::Descriptor> BaseTexture;
    // Handle<vk::Descriptor> SpecularTexture;
};
} // namespace sy::render
