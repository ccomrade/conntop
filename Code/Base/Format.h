#pragma once

#include <cstdarg>
#include <cstddef>
#include <string>

// use std::format from C++20 instead when it becomes available

#ifdef __GNUC__
#define FORMAT_ARGS_CHECK(...) __attribute__((format(printf, __VA_ARGS__)))
#else
#define FORMAT_ARGS_CHECK(...)
#endif

std::string Format(const char* format, ...) FORMAT_ARGS_CHECK(1, 2);
std::string FormatV(const char* format, va_list args);

std::size_t FormatTo(char* buffer, std::size_t bufferSize, const char* format, ...) FORMAT_ARGS_CHECK(3, 4);
std::size_t FormatToV(char* buffer, std::size_t bufferSize, const char* format, va_list args);
