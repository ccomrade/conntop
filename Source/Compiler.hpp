/**
 * @file
 * @brief Compiler-specific stuff.
 */

#pragma once

#define MACRO_STRING(x) #x
#define TO_STRING(x) MACRO_STRING(x)

#if defined(__GNUC__)  // compiler that implements GNU compiler extensions

  #if defined(__clang__)
  #define COMPILER_CLANG
  #define COMPILER_NAME_VERSION "Clang " __clang_version__
  #elif defined(__INTEL_COMPILER)
  #define COMPILER_INTEL
  #define COMPILER_NAME_VERSION "ICC " TO_STRING(__INTEL_COMPILER)
  #else
  #define COMPILER_GCC
  #define COMPILER_NAME_VERSION "GCC " __VERSION__
  #endif

  #ifdef __SSE2__
  #define COMPILER_HAS_SSE2
  #endif

  #ifdef __SSE4_2__
  #define COMPILER_HAS_SSE4_2
  #endif

  #define COMPILER_CONSTEXPR_STRLEN(x) __builtin_strlen(x)

  #define COMPILER_PRINTF_ARGS_CHECK(...) __attribute__((format(printf,__VA_ARGS__)))

#elif defined(_MSC_VER)  // MSVC or ICC on Windows

  #if defined(__INTEL_COMPILER)
  #define COMPILER_INTEL
  #define COMPILER_NAME_VERSION "ICC " TO_STRING(__INTEL_COMPILER)
  #else
  #define COMPILER_MSVC
  #define COMPILER_NAME_VERSION "MSVC " TO_STRING(_MSC_VER)
  #endif

  // MSVC only
  #if defined(COMPILER_MSVC) && (defined(_M_X64) || defined(_M_AMD64) || _M_IX86_FP == 2)
  #define COMPILER_HAS_SSE2
  #endif

  // ICC only
  #if defined(COMPILER_INTEL) && defined(__SSE2__)
  #define COMPILER_HAS_SSE2
  #endif

  // ICC only
  #if defined(COMPILER_INTEL) && defined(__SSE4_2__)
  #define COMPILER_HAS_SSE4_2
  #endif

  /* COMPILER_CONSTEXPR_STRLEN(x) */

  #define COMPILER_PRINTF_ARGS_CHECK(...)

#else  // unknown compiler

  #define COMPILER_UNKNOWN
  #define COMPILER_NAME_VERSION "unknown compiler"

  #define COMPILER_PRINTF_ARGS_CHECK(...)

#endif
