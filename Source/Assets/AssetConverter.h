#pragma once
#include <Core/Core.h>
#include <Assets/AssetCore.h>

namespace sy::asset::convert
{
	bool CovertTexture(const fs::path& input, const fs::path& output);
}