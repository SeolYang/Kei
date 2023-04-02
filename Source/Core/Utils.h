#pragma once
#include <PCH.h>

namespace sy
{
/************************ Strings ************************/
template <size_t BufferSize = 512>
std::wstring AnsiToWString(const std::string_view ansiString)
{
    std::array<wchar_t, BufferSize> buffer;
    MultiByteToWideChar(CP_ACP, 0, ansiString.data(), -1, buffer.data(), BufferSize);
    return std::wstring(buffer.data());
}

template <size_t BufferSize = 512>
std::string WStringToAnsi(const std::wstring_view wideString)
{
    std::array<char, BufferSize> buffer;
    WideCharToMultiByte(CP_ACP, 0, wideString.data(), -1, buffer.data(), BufferSize, NULL, NULL);
    return std::string(buffer.data());
}

/************************ Memory ************************/
constexpr size_t AlignForwardAdjustment(const size_t offset, const size_t alignment) noexcept
{
    const size_t adjustment = alignment - (offset & (alignment - 1));
    if (adjustment == alignment)
    {
        return 0;
    }

    return adjustment;
}

constexpr size_t PadSizeWithAlignment(const size_t allocSize, const size_t alignment)
{
    size_t alignedSize = allocSize;
    if (alignment > 0)
    {
        alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
    }

    return alignedSize;
}

template <typename T>
size_t SizeBytes(const std::vector<T>& vec)
{
    return sizeof(T) * vec.size();
}

template <typename T>
std::vector<char> ToBytes(const std::span<const T> data)
{
    std::vector<char> bytes;
    bytes.resize(data.size_bytes());
    memcpy(bytes.data(), data.data(), data.size_bytes());
    return bytes;
}

/************************ Helpers ************************/
template <typename T>
[[nodiscard]] bool FlagsContains(T flags, T flag) noexcept
{
    return (flags & flag) != 0;
}

template <typename T, size_t N>
constexpr size_t LengthOfArray(T (&)[N])
{
    return N;
}

template <typename T>
constexpr auto ToUnderlying(const T& val)
{
    return static_cast<std::underlying_type_t<T>>(val);
}

inline size_t QuerySizeOfStream(std::ifstream& stream)
{
    if (!stream.is_open())
    {
        return 0;
    }

    stream.seekg(0, std::ios::end);
    const size_t result = stream.tellg();
    stream.seekg(0, std::ios::beg);
    return result;
}

inline std::string InputFileStreamToString(const std::ifstream& stream)
{
    std::stringstream ss;
    ss << stream.rdbuf();
    return ss.str();
}

using TypeHashType = std::size_t;

/** https://stackoverflow.com/questions/56292104/hashing-types-at-compile-time-in-c17-c2a */
template <typename T>
consteval TypeHashType Hash()
{
    TypeHashType result{};

#ifdef _MSC_VER
#    define _FUNC_SIG_FOR_TYPE_HASH_ __FUNCSIG__
#else
#    define _FUNC_SIG_FOR_TYPE_HASH_ __PRETTRY_FUNCTION__
#endif

    for (const auto& c : _FUNC_SIG_FOR_TYPE_HASH_)
    {
        (result ^= c) <<= 1;
    }

    return result;
}

template <typename T>
constexpr TypeHashType TypeHash = Hash<T>();

/** Files */
inline bool SaveJsonToFile(const fs::path& path, const nlohmann::json& json, const bool bReadableFormat = true, const bool bTruncExistFile = true)
{
    if (!path.empty())
    {
        std::ofstream output{path, std::ios::out | (bTruncExistFile ? std::ios::trunc : std::ios::app)};
        if (output.is_open())
        {
            output << json.dump(bReadableFormat ? 4 : -1);
            output.close();
            return true;
        }
        else
        {
            SY_ASSERT(false, "Failed to open output stream to {}", path.string());
        }
    }
    else
    {
        SY_ASSERT(false, "Trying to save to unspecified path.");
    }

    return false;
}

inline json LoadJsonFromFile(const fs::path& path)
{
    if (!path.empty())
    {
        std::ifstream input{path, std::ios::in};
        if (input.is_open())
        {
            std::stringstream ss;
            ss << input.rdbuf();
            return json::parse(ss.str());
        }
        else
        {
            SY_ASSERT(false, "Failed to open input stream from {}", path.string());
        }
    }
    else
    {
        SY_ASSERT(false, "Trying to laod from unspecified path.");
    }

    return {};
}

inline void SaveBlobToFile(const fs::path& path, const std::span<const uint8_t> blob)
{
	if (blob.size() > 0 && !path.empty())
	{
        std::ofstream output{path, std::ios::out | std::ios::trunc | std::ios::binary};
		if (!output.is_open())
		{
            spdlog::error("Failed to open {}", path.string());
		}
		else
		{
            output.write(reinterpret_cast<const char*>(blob.data()), blob.size());
            output.close();
		}
	}
}

inline std::vector<uint8_t> LoadBlobFromFile(const fs::path& path)
{
    std::vector<uint8_t> blob;

	if (!path.empty())
	{
        std::ifstream input{path, std::ios::in | std::ios::binary};
        if (!input.is_open())
        {
            spdlog::error("Failed to open {}", path.string());
        }
        else
        {
            input.seekg(0, std::ios::end);
            const size_t sizeAsBytes = input.tellg();
            input.seekg(0, std::ios::beg);

            blob.resize(sizeAsBytes);
            input.read(reinterpret_cast<char*>(blob.data()), blob.size());
            input.close();
        }
	}

    return blob;
}

template <typename T>
T ResolveEnumFromJson(const nlohmann::json& json, const std::string_view key, const T fallback)
{
    const auto itr = json.find(key);

	if (itr == json.end())
	{
        spdlog::error("Failed to retrive enumeration from json[key:{}], return fallback value: {}.", key, magic_enum::enum_name<T>(fallback));
		return fallback;
	}

    const std::string enumStr = *itr;
    const auto        enumOpt = magic_enum::enum_cast<T>(enumStr);
	if (!enumOpt)
	{
        spdlog::error("Failed to cast {} to given enum type {}.", enumStr, magic_enum::enum_type_name<T>());
        return fallback;
	}

	return *enumOpt;
}

template <typename T>
T ResolveValueFromJson(const json& json, const std::string_view key, const T fallback)
{
    const auto itr = json.find(key);

    if (itr == json.end())
    {
        spdlog::error("Failed to find value from json[key:{}], return fallback value: {}.", key, fallback);
        return fallback;
    }

	return *itr;
}

template <typename T>
inline auto VecToConstSpan(const std::vector<T>& target)
{
    return std::span<const T>{reinterpret_cast<const std::remove_const_t<T>*>(target.data()), target.size()};
}

inline size_t ImageBlobBytesSize(const size_t width, const size_t height, const size_t channels, const size_t bytesPerChannel)
{
    return width * height * channels * bytesPerChannel;
}
} // namespace sy
