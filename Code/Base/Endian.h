#pragma once

#include <cstdint>
#include <type_traits>

enum class Endian
{
	LITTLE,
	BIG,

#if defined(__LITTLE_ENDIAN__)
	NATIVE = LITTLE
#elif defined(__BIG_ENDIAN__)
	NATIVE = BIG
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	NATIVE = LITTLE
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	NATIVE = BIG
#elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_AMD64))
	NATIVE = LITTLE
#else
#error "Unable to determine native byte order!"
#endif
};

///////////////////////////////////////////////////
// Low-level functions for endianness conversion //
///////////////////////////////////////////////////

// GCC 6+ and Clang 5+ have constexpr bswap builtin
#if (defined(__GNUC__) && __GNUC__ >= 6 && !defined(__INTEL_COMPILER)) || (defined(__clang__) && __clang_major__ >= 5)
#define ENDIAN_HAS_CONSTEXPR_BSWAP_BUILTINS
#endif

inline constexpr std::uint16_t SwapEndian16(std::uint16_t value)
{
#ifdef ENDIAN_HAS_CONSTEXPR_BSWAP_BUILTINS
	return __builtin_bswap16(value);
#else
	return (value >> 8) | (value << 8);
#endif
}

inline constexpr std::uint32_t SwapEndian32(std::uint32_t value)
{
#ifdef ENDIAN_HAS_CONSTEXPR_BSWAP_BUILTINS
	return __builtin_bswap32(value);
#else
	const std::uint32_t hi = SwapEndian16(value);
	const std::uint32_t lo = SwapEndian16(value >> 16);

	return (hi << 16) | lo;
#endif
}

inline constexpr std::uint64_t SwapEndian64(std::uint64_t value)
{
#ifdef ENDIAN_HAS_CONSTEXPR_BSWAP_BUILTINS
	return __builtin_bswap64(value);
#else
	const std::uint64_t hi = SwapEndian32(value);
	const std::uint64_t lo = SwapEndian32(value >> 32);

	return (hi << 32) | lo;
#endif
}

/////////////////////////////////
// Generic SwapEndian function //
/////////////////////////////////

template<class T, unsigned int Size>
struct SwapEndianTraits;

template<class T>
struct SwapEndianTraits<T, 1>
{
	static constexpr T Swap(T value)
	{
		// nothing to do
		return value;
	}
};

template<class T>
struct SwapEndianTraits<T, 2>
{
	static constexpr T Swap(T value)
	{
		return SwapEndian16(value);
	}
};

template<class T>
struct SwapEndianTraits<T, 4>
{
	static constexpr T Swap(T value)
	{
		return SwapEndian32(value);
	}
};

template<class T>
struct SwapEndianTraits<T, 8>
{
	static constexpr T Swap(T value)
	{
		return SwapEndian64(value);
	}
};

template<class T>
inline constexpr T SwapEndian(T value)
{
	static_assert(std::is_integral_v<T>);

	return SwapEndianTraits<T, sizeof(T)>::Swap(value);
}

/////////////////////////////////////////////////////
// Conversion to and from little-endian byte order //
/////////////////////////////////////////////////////

template<class T>
inline constexpr T LittleEndian(T value)
{
	if constexpr (Endian::NATIVE == Endian::LITTLE)
		return value;
	else
		return SwapEndian(value);
}

//////////////////////////////////////////////////
// Conversion to and from big-endian byte order //
//////////////////////////////////////////////////

template<class T>
inline constexpr T BigEndian(T value)
{
	if constexpr (Endian::NATIVE == Endian::BIG)
		return value;
	else
		return SwapEndian(value);
}
