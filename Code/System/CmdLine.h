#pragma once

#include <string>
#include <string_view>
#include <vector>

class CmdLine
{
public:
	struct Option
	{
		std::string_view name;
		std::string_view value;
	};

	std::vector<Option> options;
	std::vector<std::string_view> operands;

	CmdLine() = default;

	explicit CmdLine(int argc, char** argv)
	{
		Parse(argc, argv);
	}

	void Parse(int argc, char** argv);

private:
	enum class OptionParseResult
	{
		NOT_AN_OPTION, SHORT_OPTION, LONG_OPTION, LONG_OPTION_WITH_VALUE, END_OF_OPTIONS
	};

	OptionParseResult ParseAndPushOption(const std::string_view& arg);

public:
	// returns value of the option
	std::string_view PopRequiredOptionWithRequiredValue(const std::string_view& name);
	std::string_view PopRequiredOptionWithRequiredValue(const std::string_view& name, const std::string_view& name2);
	std::string_view PopRequiredOptionWithOptionalValue(const std::string_view& name);
	std::string_view PopRequiredOptionWithOptionalValue(const std::string_view& name, const std::string_view& name2);

	bool TryPopOptionWithoutValue(const std::string_view& name);
	bool TryPopOptionWithoutValue(const std::string_view& name, const std::string_view& name2);

	bool TryPopOptionWithRequiredValue(const std::string_view& name, std::string_view& value);
	bool TryPopOptionWithRequiredValue(const std::string_view& name, const std::string_view& name2, std::string_view& value);
	bool TryPopOptionWithOptionalValue(const std::string_view& name, std::string_view& value);
	bool TryPopOptionWithOptionalValue(const std::string_view& name, const std::string_view& name2, std::string_view& value);

	// pops all occurrences of the option at once and returns the count
	unsigned int CountAndPopOption(const std::string_view& name);
	unsigned int CountAndPopOption(const std::string_view& name, const std::string_view& name2);

	// option names are always without hyphens, use these functions to add them
	static std::string AddHyphens(const std::string_view& name);
	static std::string AddHyphens(const std::string_view& name, const std::string_view& name2);
};
