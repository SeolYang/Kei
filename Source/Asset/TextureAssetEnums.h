#pragma once

namespace sy::asset
{
enum class ETextureCompressionMode
{
	BC1,
	BC3,
    BC4,  /** Recommend to 'Gray-scale' */
    BC5,  /** Recommend to 'Normal Map' */
    BC7,  /** Recommend to 'Texture' */
    None,
};

enum class ETextureCompressionQuality
{
    Lowest,
    Low,
    Medium,
    High,
    Highest
};

inline uint32_t TextureCompressionQualityToLevel(const ETextureCompressionQuality quality)
{
    switch (quality)
    {
        case ETextureCompressionQuality::Lowest:
            return 1;
        case ETextureCompressionQuality::Low:
            return 2;
        case ETextureCompressionQuality::Medium:
            return 3;
        case ETextureCompressionQuality::High:
            return 4;
        case ETextureCompressionQuality::Highest:
            return 5;
    }

    return 2;
}

enum class ETextureQuality
{
    Low,
    Medium,
    High
};

inline uint32_t TextureQualityToLevel(const ETextureQuality quality)
{
    switch (quality)
    {
        case ETextureQuality::Low:
            return 1;
        case ETextureQuality::Medium:
            return 128;
        case ETextureQuality::High:
            return 255;
    }

    return 128;
}
} // namespace sy::asset
