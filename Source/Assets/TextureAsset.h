#pragma once
#include <Core/Core.h>
#include <Assets/AssetCore.h>

namespace sy::asset::texture
{
	enum class EFormat : uint32_t
	{
		Unknown = 0,
		RGBA8,
		HDR,
	};

	enum class EExtension
	{
		PNG,
		JPEG,
		JPG,
		HDR,
		Unknown
	};

	EFormat FormatToExtension(const EExtension extension);

	struct Metadata
	{
		uint64_t BufferSize = 0;
		EFormat Format = EFormat::Unknown;
		ECompressionMode CompressionMode = ECompressionMode::LZ4;
		Extent3D<uint32_t> Extent = { 1, 1, 1 };
		std::string SrcPath;
	};

	std::optional<Metadata> ParseMetadata(Asset& asset);
	std::optional<Asset> Pack(Metadata& metadata, void* pixelData);
	std::vector<char> Unpack(Metadata& metada, std::span<const char> src);

}