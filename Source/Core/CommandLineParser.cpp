#include <PCH.h>
#include <Core/CommandLineParser.h>

namespace sy
{
CommandLineParser::CommandLineParser(int argc, char** argv)
{
    assert(argc > 0);

    // parentPath/executable.exe
    executablePath = std::filesystem::path{argv[0]}.parent_path();
    assetPath      = executablePath / "Assets";

    for (int idx = 1; idx < argc; ++idx)
    {
        if (const auto bIsInvalidArgument = !Argument(argv[idx]); bIsInvalidArgument)
        {
            spdlog::warn("Invalid argument {}.", argv[idx]);
        }
    }
}

bool CommandLineParser::Argument(const char* argument)
{
    if (lstrcmpA(argument, "-import_assets") == 0)
    {
        spdlog::info("Enabled: Import Assets");
        bImportAssets = true;
        return true;
    }

	if (lstrcmpA(argument, "-force_reimport_assets") == 0)
	{
        spdlog::info("Enabled: Force re-import assets");
        bForceReimportAssets = true;
        return true;
	}

    return false;
}
} // namespace sy
