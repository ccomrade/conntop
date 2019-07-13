/**
 * @brief Configuration of RapidJSON library and its forward declarations.
 * This file must be included before anything else from the rapidjson directory.
 */

#pragma once

#include "Types.hpp"  // size_t
#include "Compiler.hpp"
#include "conntop_config.h"

#define RAPIDJSON_HAS_STDSTRING 0

#define RAPIDJSON_48BITPOINTER_OPTIMIZATION 0

#ifdef CONNTOP_MACHINE_IS_BIG_ENDIAN
#define RAPIDJSON_ENDIAN RAPIDJSON_BIGENDIAN
#else
#define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN
#endif

#ifdef COMPILER_HAS_SSE2
#define RAPIDJSON_SSE2
#endif

#ifdef COMPILER_HAS_SSE4_2
#define RAPIDJSON_SSE42
#endif

#ifdef COMPILER_HAS_ARM_NEON
#define RAPIDJSON_NEON
#endif

#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson
{
	using SizeType = size_t;
}

// forward declarations
#include "rapidjson/fwd.h"
