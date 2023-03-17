#pragma once
#include <PCH.h>

namespace sy
{
	/************************ Strings ************************/
	template <size_t BufferSize = 512>
	std::wstring AnsiToWString( const std::string_view ansiString )
	{
		std::array<wchar_t, BufferSize> buffer;
		MultiByteToWideChar(CP_ACP, 0, ansiString.data(), -1, buffer.data(), BufferSize);
		return std::wstring(buffer.data());
	}

	template <size_t BufferSize = 512>
	std::string WStringToAnsi( const std::wstring_view wideString )
	{
		std::array<char, BufferSize> buffer;
		WideCharToMultiByte(CP_ACP, 0, wideString.data(), -1, buffer.data(), BufferSize, NULL, NULL);
		return std::string(buffer.data());
	}

	/************************ Memory ************************/
	constexpr size_t AlignForwardAdjustment( const size_t offset, const size_t alignment ) noexcept
	{
		const size_t adjustment = alignment - (offset & (alignment - 1));
		if (adjustment == alignment)
		{
			return 0;
		}

		return adjustment;
	}

	constexpr size_t PadSizeWithAlignment( const size_t allocSize, const size_t alignment )
	{
		size_t alignedSize = allocSize;
		if (alignment > 0)
		{
			alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
		}

		return alignedSize;
	}

	template <typename T>
	size_t SizeBytes( const std::vector<T>& vec )
	{
		return sizeof(T) * vec.size();
	}

	template <typename T>
	std::vector<char> ToBytes( const std::span<const T> data )
	{
		std::vector<char> bytes;
		bytes.resize(data.size_bytes());
		memcpy(bytes.data(), data.data(), data.size_bytes());
		return bytes;
	}

	/************************ Helpers ************************/
	template <typename T>
	[[nodiscard]] bool FlagsContains( T flags, T flag ) noexcept
	{
		return (flags & flag) != 0;
	}

	template <typename T, size_t N>
	constexpr size_t LengthOfArray( T (&)[ N ] )
	{
		return N;
	}

	template <typename T>
	constexpr auto ToUnderlying( const T& val )
	{
		return static_cast<std::underlying_type_t<T>>(val);
	}

	inline size_t QuerySizeOfStream( std::ifstream& stream )
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

	inline std::string InputFileStreamToString( const std::ifstream& stream )
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
#define _FUNC_SIG_FOR_TYPE_HASH_ __FUNCSIG__
#else
#define _FUNC_SIG_FOR_TYPE_HASH_ __PRETTRY_FUNCTION__
#endif

		for (const auto& c : _FUNC_SIG_FOR_TYPE_HASH_)
		{
			(result ^= c) <<= 1;
		}

		return result;
	}

	template <typename T>
	constexpr TypeHashType TypeHash = Hash<T>();
}
