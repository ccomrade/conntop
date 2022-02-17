#include <cstdio>

#include "Format.h"

std::string Format(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::string result = FormatV(format, args);
	va_end(args);

	return result;
}

std::string FormatV(const char* format, va_list args)
{
	char buffer[1024];
	std::size_t length = FormatToV(buffer, sizeof buffer, format, args);

	std::string result;

	// make sure the resulting string is not truncated
	if (length < sizeof buffer)
	{
		result.assign(buffer, length);
	}
	else
	{
		result.resize(length);

		// format string again with proper buffer size
		length = FormatToV(result.data(), result.length() + 1, format, args);

		if (length < result.length())
			result.resize(length);
	}

	return result;
}

std::size_t FormatTo(char* buffer, std::size_t bufferSize, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	std::size_t length = FormatToV(buffer, bufferSize, format, args);
	va_end(args);

	return length;
}

std::size_t FormatToV(char* buffer, std::size_t bufferSize, const char* format, va_list args)
{
	if (buffer && bufferSize && format)
	{
		// make sure the argument list is never modified
		va_list argsCopy;
		va_copy(argsCopy, args);
		int status = std::vsnprintf(buffer, bufferSize, format, argsCopy);
		va_end(argsCopy);

		return (status > 0) ? status : 0;
	}
	else
	{
		return 0;
	}
}
