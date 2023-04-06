#pragma once
#include <PCH.h>

namespace sy
{
class RawImage : public NonCopyable
{
private:
	using RawImageDataUniquePtr = std::unique_ptr<uint8_t, std::function<void(uint8_t*)>>;

public:
    RawImage() = default;
    ~RawImage() = default;

    [[nodiscard]] bool IsHDR() const { return bIsHighDynamicRangeImage; }
    [[nodiscard]] uint8_t GetNumChannels() const { return numChannels; }
    [[nodiscard]] Extent3D<uint32_t> GetExtent() const { return extent; }
    [[nodiscard]] size_t GetBytesPerChannel() const { return bytesPerChannel; }
    [[nodiscard]] size_t GetBytesPerPixel() const { return static_cast<size_t>(GetNumChannels()) * GetBytesPerChannel(); }
    [[nodiscard]] size_t GetSizeBytes() const { return GetBytesPerPixel() * extent.width * extent.height * extent.depth; }
    [[nodiscard]] VkFormat GetEstimatedFormat() const { return vk::EstimateFormat(GetNumChannels(), GetBytesPerChannel()); }

    [[nodiscard]] const uint8_t* GetData() const { return data.get(); }
    [[nodiscard]] std::span<const uint8_t> GetDataSpan() const { return std::span{reinterpret_cast<const uint8_t*>(data.get()), GetSizeBytes()}; }

    bool LoadFromFile(const fs::path& path);

private:
    void AcquireImageMetadata(std::string_view pathStr);
    void LoadImageDataFromFile(std::string_view pathStr);

private:
    RawImageDataUniquePtr data;
    bool bIsHighDynamicRangeImage = false;
    size_t bytesPerChannel = 0;
    uint8_t numChannels = 0;
    Extent3D<uint32_t> extent{1, 1, 1};
};
} // namespace sy
