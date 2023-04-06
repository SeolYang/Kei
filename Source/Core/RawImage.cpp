#include <PCH.h>
#include <Core/RawImage.h>

namespace sy
{
bool RawImage::LoadFromFile(const fs::path& path)
{
    const std::string pathStr = path.string();
    if (!fs::exists(path))
    {
        spdlog::error("File {} does not exist.", pathStr);
        return false;
    }

    AcquireImageMetadata(pathStr);
    LoadImageDataFromFile(pathStr);

    return data != nullptr;
}

void RawImage::AcquireImageMetadata(std::string_view pathStr)
{
    if (stbi_is_16_bit(pathStr.data()))
    {
        bytesPerChannel = 2;
    }
    else if (stbi_is_hdr(pathStr.data()))
    {
        bIsHighDynamicRangeImage = true;
        bytesPerChannel = 4;
    }
    else
    {
        bytesPerChannel = 1;
    }

    int width = 0;
    int height = 0;
    int numChannels = 0;
    stbi_info(pathStr.data(), &width, &height, &numChannels);
    this->extent = Extent3D<uint32_t>{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    this->numChannels = static_cast<size_t>(numChannels);
}

void RawImage::LoadImageDataFromFile(std::string_view pathStr)
{
    static constexpr auto StbiDeleter = [](uint8_t* ptr) {
        stbi_image_free(ptr);
    };

	int dummy;
    switch (bytesPerChannel)
    {
        case 1:
            data = RawImageDataUniquePtr(reinterpret_cast<uint8_t*>(stbi_load(pathStr.data(), &dummy, &dummy, &dummy, 0)),
                                         StbiDeleter);
            break;
        case 2:
            data = RawImageDataUniquePtr(reinterpret_cast<uint8_t*>(stbi_load_16(pathStr.data(), &dummy, &dummy, &dummy, 0)),
                                         StbiDeleter);
            break;
        case 4:
            data = RawImageDataUniquePtr(reinterpret_cast<uint8_t*>(stbi_loadf(pathStr.data(), &dummy, &dummy, &dummy, 0)),
                                         StbiDeleter);
            break;
    }
}
} // namespace sy