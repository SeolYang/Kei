#pragma once
#include <Core/Core.h>

namespace sy
{
	class CommandLineParser
	{
	public:
		CommandLineParser(int argc, char** argv);

		[[nodiscard]] auto GetExecutablePath() const  noexcept { return executablePath; }
		[[nodiscard]] auto GetAssetPath() const  noexcept { return assetPath; }
		[[nodiscard]] auto ShouldConvertAssets() const  noexcept { return bShouldConvertAssets; }

	private:
		bool Argument(const char* argument);

	private:
		fs::path executablePath;
		fs::path assetPath;
		bool bShouldConvertAssets = false;

	};
}
