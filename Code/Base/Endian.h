#pragma once

#include <stdint.h>
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

inline constexpr uint16_t SwapEndian16(uint16_t value)
{
#ifdef ENDIAN_HAS_CONSTEXPR_BSWAP_BUILTINS
	return __builtin_bswap16(value);
#else
	return (value >> 8) | (value << 8);
#endif
}

inline constexpr uint32_t SwapEndian32(uint32_t value)
{
#ifdef ENDIAN_HAS_CONSTEXPR_BSWAP_BUILTINS
	return __builtin_bswap32(value);
#else
	const uint32_t hi = SwapEndian16(value);
	const uint32_t lo = SwapEndian16(value >> 16);

	return (hi << 16) | lo;
#endif
}

inline constexpr uint64_t SwapEndian64(uint64_t value)
{
#ifdef ENDIAN_HAS_CONSTEXPR_BSWAP_BUILTINS
	return __builtin_bswap64(value);
#else
	const uint64_t hi = SwapEndian32(value);
	const uint64_t lo = SwapEndian32(value >> 32);

	return (hi << 32) | lo;
#endif
}

/////////////////////////////////
// Generic SwapEndian function //
/////////////////////////////////

template<class T, unsigned int Size>
struct SwapEndianTraits;

template<class T>
struct SwapEndianTraits<T, 1>  // 8-bit
{
	static constexpr T Swap(T value)
	{
		// nothing to do
		return value;
	}
};

template<class T>
struct SwapEndianTraits<T, 2>  // 16-bit
{
	static constexpr T Swap(T value)
	{
		return SwapEndian16(value);
	}
};

template<class T>
struct SwapEndianTraits<T, 4>  // 32-bit
{
	static constexpr T Swap(T value)
	{
		return SwapEndian32(value);
	}
};

template<class T>
struct SwapEndianTraits<T, 8>  // 64-bit
{
	static constexpr T Swap(T value)
	{
		return SwapEndian64(value);
	}
};

template<class T>
inline constexpr T SwapEndian(T value)
{
	static_assert(std::is_integral<T>(), "The value must be an integral type!");

	return SwapEndianTraits<T, sizeof(T)>::Swap(value);
}

/////////////////////////////////////////////////////
// Conversion to and from little-endian byte order //
/////////////////////////////////////////////////////

template<class T>
inline constexpr T LittleEndian(T value)
{
	return (Endian::NATIVE == Endian::LITTLE) ? value : SwapEndian(value);
}

//////////////////////////////////////////////////
// Conversion to and from big-endian byte order //
//////////////////////////////////////////////////

template<class T>
inline constexpr T BigEndian(T value)
{
	return (Endian::NATIVE == Endian::BIG) ? value : SwapEndian(value);
}
