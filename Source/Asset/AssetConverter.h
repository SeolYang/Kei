#pragma once
#include <Core/Core.h>
#include <Asset/AssetCore.h>

namespace sy::asset
{
	bool ConvertTexture2D(const fs::path& input);
	bool ConvertTexture2D(const fs::path& input, const fs::path& output);
}