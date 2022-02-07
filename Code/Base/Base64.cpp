#include <array>

#include "Base64.h"

namespace
{
	constexpr std::string_view ENCODING_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	static_assert(ENCODING_TABLE.size() == 64);

	constexpr unsigned char INVALID_CHARACTER = 0xFF;
	constexpr std::array<unsigned char, 256> DECODING_TABLE = []()
	{
		std::array<unsigned char, 256> table = {};

		for (unsigned char& ch : table)
		{
			ch = INVALID_CHARACTER;
		}

		for (unsigned char i = 0; i < 64; i++)
		{
			table[ENCODING_TABLE[i]] = i;
		}

		return table;
	}();
}

std::string Base64::Encode(const std::string_view& text)
{
	const std::string::size_type resultLength = ((text.length() + 2) / 3) * 4;

	// size_type overflow
	if (resultLength < text.length())
	{
		throw std::length_error("Data is too long to be encoded as Base64");
	}

	std::string result;
	result.resize(resultLength);
	auto output = result.data();

	auto input = reinterpret_cast<const unsigned char*>(text.data());
	auto inputEnd = input + text.length();

	for (; (inputEnd - input) >= 3; input += 3)
	{
		*output++ = ENCODING_TABLE[input[0] >> 2];
		*output++ = ENCODING_TABLE[((input[0] & 0x03) << 4) | (input[1] >> 4)];
		*output++ = ENCODING_TABLE[((input[1] & 0x0F) << 2) | (input[2] >> 6)];
		*output++ = ENCODING_TABLE[input[2] & 0x3F];
	}

	if ((inputEnd - input) > 0)
	{
		*output++ = ENCODING_TABLE[input[0] >> 2];

		if ((inputEnd - input) == 1)
		{
			*output++ = ENCODING_TABLE[(input[0] & 0x03) << 4];
			*output++ = '=';
		}
		else
		{
			*output++ = ENCODING_TABLE[((input[0] & 0x03) << 4) | (input[1] >> 4)];
			*output++ = ENCODING_TABLE[(input[1] & 0x0F) << 2];
		}

		*output++ = '=';
	}

	return result;
}

std::string Base64::Decode(const std::string_view& text)
{
	if (text.length() % 4)
	{
		throw std::invalid_argument("Base64 string length must be a multiple of 4");
	}

	const std::string::size_type resultLength = (text.length() / 4) * 3;

	std::string result;
	result.resize(resultLength);
	auto output = reinterpret_cast<unsigned char*>(result.data());

	auto input = reinterpret_cast<const unsigned char*>(text.data());
	auto inputEnd = input + text.length();

	unsigned int padding = 0;

	for (; input < inputEnd; input += 4)
	{
		std::array<unsigned char, 4> block = {};

		for (int i = 0; i < 4; i++)
		{
			block[i] = DECODING_TABLE[input[i]];
		}

		for (int i = 0; i < 4; i++)
		{
			if (block[i] == INVALID_CHARACTER)
			{
				const auto ch = input[i];
				const auto nextCh = input[i + 1];

				const bool isPadding = (ch == '=');
				const bool isPaddingValid = ((i == 2 && nextCh == '=') || i == 3);
				const bool isLastBlock = (input == (inputEnd - 4));

				if (isPadding && isPaddingValid && isLastBlock)
				{
					padding++;
					block[i] = 0;
				}
				else
				{
					throw std::invalid_argument("Invalid Base64 character " + std::to_string(ch));
				}
			}
		}

		*output++ = static_cast<unsigned char>((block[0] << 2) | (block[1] >> 4));
		*output++ = static_cast<unsigned char>((block[1] << 4) | (block[2] >> 2));
		*output++ = static_cast<unsigned char>((block[2] << 6) | (block[3]));
	}

	result.resize(result.size() - padding);

	return result;
}
