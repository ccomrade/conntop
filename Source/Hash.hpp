/**
 * @file
 * @brief Hash functions.
 */

#pragma once

#include <functional>  // std::hash

#include "Types.hpp"

/**
 * @brief Combines two hash values.
 * Implementation is taken from the Boost library. It uses MurmurHash3.
 * @param seed Reference to the first hash value. Result is stored there.
 * @param hash The second hash value.
 */
inline void HashCombine(size_t & seed, size_t hash)
{
#if SIZE_MAX == UINT32_MAX  // 32-bit size_t

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	hash *= c1;
	hash = (hash << 15) | (hash >> (32 - 15));
	hash *= c2;

	seed ^= hash;
	seed = (seed << 13) | (seed >> (32 - 13));
	seed = seed*5 + 0xe6546b64;

#elif SIZE_MAX == UINT64_MAX  // 64-bit size_t

	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;

	hash *= m;
	hash ^= hash >> r;
	hash *= m;

	seed ^= hash;
	seed *= m;
	seed += 0xe6546b64;

#else

	#error "Invalid size_t length"

#endif
}
