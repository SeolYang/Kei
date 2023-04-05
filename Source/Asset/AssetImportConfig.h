#pragma once
#include <PCH.h>
#include <Core/Serializable.h>

namespace sy::asset
{
class AssetImportConfig : public Serializable
{
public:
	[[nodiscard]] bool IsReadyToImport() const { return bReadyToImport; }
    void               SetReadyToImport(const bool bReadyToImport) { this->bReadyToImport = bReadyToImport; }

	json Serialize() const override;
    void Deserialize(const json& root) override;

protected:
    AssetImportConfig() = default;

private:
	bool bReadyToImport = true;
};
} // namespace sy::asset