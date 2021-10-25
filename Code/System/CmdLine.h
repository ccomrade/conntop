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

		std::string GetPrettyName() const
		{
			return CmdLine::PrettifyOption(name);
		}
	};

	enum class OptionType
	{
		NOT_AN_OPTION, SHORT_OPTION, LONG_OPTION, LONG_OPTION_WITH_VALUE, END_OF_OPTIONS
	};

private:
	std::vector<Option> m_options;
	std::vector<std::string_view> m_operands;

public:
	CmdLine() = default;

	explicit CmdLine(int argc, char** argv)
	{
		Parse(argc, argv);
	}

	void Parse(int argc, char** argv);

	OptionType ParseAndPushOption(const std::string_view& arg);

	void PushOption(const std::string_view& name, const std::string_view& value = {});
	void PushOperand(const std::string_view& operand);

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

	// pops all occurrences of the option at once
	unsigned int CountAndPopOption(const std::string_view& name);
	unsigned int CountAndPopOption(const std::string_view& name, const std::string_view& name2);

	void Clear();

	std::vector<Option>& GetOptions()
	{
		return m_options;
	}

	std::vector<std::string_view>& GetOperands()
	{
		return m_operands;
	}

	const std::vector<Option>& GetOptions() const
	{
		return m_options;
	}

	const std::vector<std::string_view>& GetOperands() const
	{
		return m_operands;
	}

	static std::string PrettifyOption(const std::string_view& name);
	static std::string PrettifyOption(const std::string_view& name, const std::string_view& name2);
};

//////////////////////////
// Comparison operators //
//////////////////////////

inline bool operator==(const CmdLine::Option& a, const CmdLine::Option& b)
{
	return a.name == b.name;
}

inline bool operator!=(const CmdLine::Option& a, const CmdLine::Option& b)
{
	return a.name != b.name;
}

inline bool operator==(const CmdLine::Option& option, const std::string_view& name)
{
	return option.name == name;
}

inline bool operator!=(const CmdLine::Option& option, const std::string_view& name)
{
	return option.name != name;
}

inline bool operator==(const std::string_view& name, const CmdLine::Option& option)
{
	return name == option.name;
}

inline bool operator!=(const std::string_view& name, const CmdLine::Option& option)
{
	return name != option.name;
}
