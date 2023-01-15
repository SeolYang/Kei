#pragma once
#include <PCH.h>

namespace sy::asset
{
	constexpr auto UNKNOWN_ASSET_IDENTIFIER = "UKUN";
	constexpr auto TEXTURE_ASSET_IDENTIFIER = "TEX_";
	constexpr auto AUDIO_ASSET_IDENTIFIER = "AUDI";
	constexpr auto MODEL_ASSET_IDENTIFIER = "MODL";

	enum class ECompressionMode : uint32_t
	{
		None,
		LZ4,
		//ETC2,
		//BC7
		/* More efficient compress algorithm for each assets. **/
	};

	enum class EAssetExtension
	{
		TEX,
		MODEL,
	};

	enum class EAssetType
	{
		Unknown,
		Texture,
		Model,
		Audio,
		Shader,
		ShaderBinary,
	};

	enum class ETextureExtension
	{
		PNG,
		JPEG,
		JPG,
		HDR,
	};

	enum class EModelExtension
	{
		FBX,
		OBJ,
		GLTF,
		GLTF2,
	};

	enum class EAudioExtension
	{
		MP3,
		WAV,
		OGG
	};

	enum class EShaderExtension
	{
		GLSL,
		HLSL,
	};

	enum class EShaderBinaryExtension
	{
		SPV,
	};


	constexpr size_t IDENTIFIER_LENGTH = 4;
	struct Asset
	{
		char Identifier[IDENTIFIER_LENGTH] = { 0, 0, 0, 0 };
		uint32_t Version = 0;
		std::string Metadata = "{}"; // Json
		std::vector<char> Blob;
	};

	inline EAssetType FileExtensionToAssetType(const std::string& extension)
	{
		static std::once_flag initTableFlag;
		static robin_hood::unordered_map<std::string, EAssetType> ExtensionTable;

		std::call_once(initTableFlag,
			[&]()
			{
				for (const std::string_view name : magic_enum::enum_names<ETextureExtension>())
				{
					std::string nameStr{ name };
					ExtensionTable[nameStr] = EAssetType::Texture;
				}

				for (const std::string_view name : magic_enum::enum_names<EModelExtension>())
				{
					std::string nameStr{ name };
					ExtensionTable[nameStr] = EAssetType::Model;
				}

				for (const std::string_view name : magic_enum::enum_names<EAudioExtension>())
				{
					std::string nameStr{ name };
					ExtensionTable[nameStr] = EAssetType::Audio;
				}

				for (const std::string_view name : magic_enum::enum_names<EShaderExtension>())
				{
					std::string nameStr{ name };
					ExtensionTable[nameStr] = EAssetType::Shader;
				}

				for (const std::string_view name : magic_enum::enum_names<EShaderBinaryExtension>())
				{
					std::string nameStr{ name };
					ExtensionTable[nameStr] = EAssetType::ShaderBinary;
				}
			});

		const auto foundItr = ExtensionTable.find(extension);
		if (foundItr != ExtensionTable.end())
		{
			return foundItr->second;
		}

		return EAssetType::Unknown;
	}

	bool SaveBinary(std::string_view path, const Asset& asset);
	std::optional<Asset> LoadBinary(std::string_view path, uint32_t version, bool bIgnoreAssetVersionCheck = false);

	std::vector<char> CompressData(ECompressionMode compressionMode, std::span<const char> data);
	std::vector<char> Uncompress(ECompressionMode compressionMode, size_t dataSize, std::span<const char> compressedData);

}
