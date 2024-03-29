#pragma once
#include <PCH.h>
#include <Asset/Constants.h>

namespace sy::asset
{
inline fs::path ConvertToExtensionless(fs::path path)
{
    return path.replace_extension();
}

inline fs::path ConvertToAssetPath(fs::path path)
{
    return path.replace_extension(constants::ext::Asset);
}

inline fs::path ConvertToBlobPath(fs::path path)
{
    return path.replace_extension(constants::ext::Blob);
}

class Asset : public NonCopyable, public NamedType, public Serializable
{
public:
    explicit Asset(const fs::path& path);

    virtual size_t GetTypeHash() const = 0;

    [[nodiscard]] explicit operator bool() const { return bInitialized; }

    [[nodiscard]] const fs::path& GetOriginPath() const { return originPath; }
    [[nodiscard]] const fs::path& GetPath() const { return assetPath; }
    [[nodiscard]] const fs::path& GetBlobPath() const { return blobPath; }
    [[nodiscard]] const fs::path& GetExtensionlessPath() const { return extensionlessPath; }

    bool Initialize();

    [[nodiscard]] json Serialize() const override;
    void               Deserialize(const json& root) override;

protected:
    void EnableIgnoreBlob() { bIgnoreBlob = true; }
    /** #consideration : Take into account usage of '.ktx' format for texture. */
    void MarkAsExternalFormat() { bIsExternalFormat = true; }
    void AllowUsingMetadataForExternalFormat() { bAllowUsingMetadataForExternalFormat = true; }
    void MarkAsInitialized() { bInitialized = true; }

    virtual void BeginInit() {}
    virtual void EndInit() {}
    virtual void BeginDeserialize() {}
    virtual void EndDeserialize() {}
    virtual void BeginInitBlob() {}
    virtual void EndInitBlob() {}
    virtual void BeginInitExternal() {}
    virtual void EndInitExternal() {}

    virtual bool InitializeBlob(std::vector<uint8_t> blob) { return false; }
    virtual bool InitializeExternal() { return false; }

private:
    bool           bIgnoreBlob                          = false;
    bool           bIsExternalFormat                    = false;
    bool           bAllowUsingMetadataForExternalFormat = false;
    bool           bInitialized                         = false;
    const fs::path originPath;
    const fs::path extensionlessPath;
    const fs::path assetPath;
    const fs::path blobPath;
};
} // namespace sy::asset