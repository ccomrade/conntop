#include <algorithm>
#include <stdexcept>

#include "CmdLine.h"

void CmdLine::Parse(int argc, char** argv)
{
	bool optionsEndFound = false;
	bool nextArgIsValue = false;

	for (int i = 1; i < argc; i++)
	{
		const std::string_view arg = argv[i];

		if (!optionsEndFound)
		{
			bool useNextArgAsValue = false;

			switch (ParseAndPushOption(arg))
			{
				case OptionParseResult::NOT_AN_OPTION:
					if (nextArgIsValue)
						options.back().value = arg;
					else
						operands.push_back(arg);
					break;
				case OptionParseResult::LONG_OPTION:
				case OptionParseResult::SHORT_OPTION:
					useNextArgAsValue = true;
					break;
				case OptionParseResult::LONG_OPTION_WITH_VALUE:
					break;
				case OptionParseResult::END_OF_OPTIONS:
					optionsEndFound = true;
					break;
			}

			nextArgIsValue = useNextArgAsValue;
		}
		else
		{
			operands.push_back(arg);
		}
	}
}

CmdLine::OptionParseResult CmdLine::ParseAndPushOption(const std::string_view& arg)
{
	if (arg.length() < 2 || arg[0] != '-')
	{
		return OptionParseResult::NOT_AN_OPTION;
	}

	if (arg[1] == '-')
	{
		if (arg.length() == 2)
		{
			// found "--"
			return OptionParseResult::END_OF_OPTIONS;
		}
		else
		{
			Option longOption;
			longOption.name = arg;

			// remove "--"
			longOption.name.remove_prefix(2);

			// parse "option=value"
			if (const auto separatorPos = longOption.name.find('='); separatorPos != std::string_view::npos)
			{
				longOption.value = longOption.name;

				// remove "option="
				longOption.value.remove_prefix(separatorPos + 1);
				// remove "=value"
				longOption.name.remove_suffix(longOption.name.length() - separatorPos);

				options.push_back(longOption);

				return OptionParseResult::LONG_OPTION_WITH_VALUE;
			}
			else
			{
				options.push_back(longOption);

				return OptionParseResult::LONG_OPTION;
			}
		}
	}
	else
	{
		if (arg.length() == 2)
		{
			Option shortOption;
			shortOption.name = arg;

			// remove "-"
			shortOption.name.remove_prefix(1);

			options.push_back(shortOption);
		}
		else
		{
			// parse multiple short options packed together
			for (unsigned int i = 1; i < arg.length(); i++)
			{
				Option shortOption;
				shortOption.name = arg.substr(i, 1);

				options.push_back(shortOption);
			}
		}

		return OptionParseResult::SHORT_OPTION;
	}
}

std::string_view CmdLine::PopRequiredOptionWithRequiredValue(const std::string_view& name)
{
	std::string_view value;

	if (!TryPopOptionWithRequiredValue(name, value))
	{
		throw std::runtime_error("Option '" + AddHyphens(name) + "' is missing");
	}
	else
	{
		return value;
	}
}

std::string_view CmdLine::PopRequiredOptionWithRequiredValue(const std::string_view& name, const std::string_view& name2)
{
	std::string_view value;

	if (!TryPopOptionWithRequiredValue(name, name2, value))
	{
		throw std::runtime_error("Option '" + AddHyphens(name, name2) + "' is missing");
	}
	else
	{
		return value;
	}
}

std::string_view CmdLine::PopRequiredOptionWithOptionalValue(const std::string_view& name)
{
	std::string_view value;

	if (!TryPopOptionWithOptionalValue(name, value))
	{
		throw std::runtime_error("Option '" + AddHyphens(name) + "' is missing");
	}
	else
	{
		return value;
	}
}

std::string_view CmdLine::PopRequiredOptionWithOptionalValue(const std::string_view& name, const std::string_view& name2)
{
	std::string_view value;

	if (!TryPopOptionWithOptionalValue(name, name2, value))
	{
		throw std::runtime_error("Option '" + AddHyphens(name, name2) + "' is missing");
	}
	else
	{
		return value;
	}
}

bool CmdLine::TryPopOptionWithoutValue(const std::string_view& name)
{
	std::string_view value;

	if (!TryPopOptionWithOptionalValue(name, value))
	{
		return false;
	}
	else
	{
		if (!value.empty())
		{
			// this value is actually an operand
			operands.push_back(value);
		}

		return true;
	}
}

bool CmdLine::TryPopOptionWithoutValue(const std::string_view& name, const std::string_view& name2)
{
	std::string_view value;

	if (!TryPopOptionWithOptionalValue(name, name2, value))
	{
		return false;
	}
	else
	{
		if (!value.empty())
		{
			// this value is actually an operand
			operands.push_back(value);
		}

		return true;
	}
}

bool CmdLine::TryPopOptionWithRequiredValue(const std::string_view& name, std::string_view& value)
{
	if (!TryPopOptionWithOptionalValue(name, value))
	{
		return false;
	}
	else
	{
		if (value.empty())
		{
			throw std::runtime_error("Option '" + AddHyphens(name) + "' requires a value");
		}

		return true;
	}
}

bool CmdLine::TryPopOptionWithRequiredValue(const std::string_view& name, const std::string_view& name2, std::string_view& value)
{
	if (!TryPopOptionWithOptionalValue(name, name2, value))
	{
		return false;
	}
	else
	{
		if (value.empty())
		{
			throw std::runtime_error("Option '" + AddHyphens(name, name2) + "' requires a value");
		}

		return true;
	}
}

bool CmdLine::TryPopOptionWithOptionalValue(const std::string_view& name, std::string_view& value)
{
	const auto comparator = [&name](const Option& option)
	{
		return option.name == name;
	};

	// use reverse iterators to get the last matching option
	const auto it = std::find_if(options.rbegin(), options.rend(), comparator);

	if (it == options.rend())
	{
		return false;
	}
	else
	{
		value = it->value;

		// pop the option
		// convert the reverse iterator into a forward iterator
		options.erase(std::next(it).base());

		return true;
	}
}

bool CmdLine::TryPopOptionWithOptionalValue(const std::string_view& name, const std::string_view& name2, std::string_view& value)
{
	const auto comparator = [&name, &name2](const Option& option)
	{
		return option.name == name || option.name == name2;
	};

	// use reverse iterators to get the last matching option
	const auto it = std::find_if(options.rbegin(), options.rend(), comparator);

	if (it == options.rend())
	{
		return false;
	}
	else
	{
		value = it->value;

		// pop the option
		// convert the reverse iterator into a forward iterator
		options.erase(std::next(it).base());

		return true;
	}
}

unsigned int CmdLine::CountAndPopOption(const std::string_view& name)
{
	const auto comparator = [&name](const Option& option)
	{
		return option.name == name;
	};

	const auto originalSize = options.size();

	options.erase(std::remove_if(options.begin(), options.end(), comparator), options.end());

	return originalSize - options.size();
}

unsigned int CmdLine::CountAndPopOption(const std::string_view& name, const std::string_view& name2)
{
	return CountAndPopOption(name) + CountAndPopOption(name2);
}

std::string CmdLine::AddHyphens(const std::string_view& name)
{
	if (name.length() == 1)
	{
		return "-" + std::string(name);
	}
	else if (name.length() > 1)
	{
		return "--" + std::string(name);
	}
	else
	{
		return {};
	}
}

std::string CmdLine::AddHyphens(const std::string_view& name, const std::string_view& name2)
{
	return AddHyphens(name) + "|" + AddHyphens(name2);
}
