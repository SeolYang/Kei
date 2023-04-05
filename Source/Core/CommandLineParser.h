#pragma once
#include <PCH.h>

namespace sy
{
class CommandLineParser final : public NonCopyable
{
public:
    CommandLineParser(int argc, char** argv);

    [[nodiscard]] auto GetExecutablePath() const noexcept
    {
        return executablePath;
    }

    [[nodiscard]] auto GetAssetPath() const noexcept
    {
        return assetPath;
    }

    [[nodiscard]] auto IsImportAssetEnabled() const noexcept
    {
        return bImportAssets;
    }

    [[nodiscard]] auto IsForceReimportAssetsEnabled() const noexcept
    {
        return bForceReimportAssets;
    }


private:
    bool Argument(const char* argument);

private:
    fs::path executablePath;
    fs::path assetPath;
    bool     bImportAssets        = false;
    bool     bForceReimportAssets = false;
};
} // namespace sy
