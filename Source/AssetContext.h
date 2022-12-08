#pragma once
#include <Core.h>

namespace sy
{
	namespace asset
	{
		constexpr auto TEXTURE_ASSET_IDENTIFIER = "TEX_";
		constexpr auto AUDIO_ASSET_IDENTIFIER = "AUDI";
		constexpr auto MESH_ASSET_IDENTIFIER = "MESH";
	}

	class AssetContext
	{
	public:
		struct Asset
		{
			char TypeIdentifier[4] = {'U', 'K', 'U', 'N'};
			uint32_t Version = 0;
			std::string Metadata = "{}"; // Json
			std::vector<char> Blob;
		};

	public:
		static bool SaveBinary(std::string_view path, const Asset& asset);
		static std::optional<Asset> LoadBinary(std::string_view path);

	};
}