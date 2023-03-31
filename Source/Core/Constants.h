#pragma once
#include <string_view>

namespace sy::core::constants::res
{
/** Predefined Engine Resources */
constexpr std::string_view DefaultWhiteTexture     = "Engine/WhiteTexture";
constexpr std::string_view DefaultBlackTexture     = "Engine/BlackTexture";
constexpr std::string_view DefaultMaterialInstance = "Engine/DefaultMaterialInstance";
/** Min: Linear, Mag: Linear, Mip: Linear, AddressModeUVW = Repeat */
constexpr std::string_view TrilinearRepeatSampler  = "Engine/TrilinearRepeatSampler";
}
