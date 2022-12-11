#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace asset
	{
		constexpr auto UNKNOWN_ASSET_IDENTIFIER = "UKUN";
		constexpr auto TEXTURE_ASSET_IDENTIFIER = "TEX_";
		constexpr auto AUDIO_ASSET_IDENTIFIER = "AUDI";
		constexpr auto MESH_ASSET_IDENTIFIER = "MESH";

		enum class ECompressionMode : uint32_t
		{
			None,
			LZ4,
			/* More efficient compress algorithm for each assets. **/
		};

		struct Asset
		{
			char Identifier[4] = { 0, 0, 0, 0 };
			uint32_t Version = 0;
			std::string Metadata = "{}"; // Json
			std::vector<char> Blob;
		};

		bool SaveBinary(std::string_view path, const Asset& asset);
		std::optional<Asset> LoadBinary(std::string_view path);
	}
}
