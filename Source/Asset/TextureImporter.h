#pragma once
#include <PCH.h>

namespace sy::asset
{
struct TextureImportConfig
{
};


class TextureImporter
{
public:
    static bool Import(const fs::path& path, TextureImportConfig config);

private:
    TextureImporter()  = default;
    ~TextureImporter() = default;
};
} // namespace sy::asset