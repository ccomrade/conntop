#include <algorithm>
#include <stdexcept>

#include "CmdLine.h"

void CmdLine::Parse(int argc, char** argv)
{
	bool parseOptions = true;
	bool attachValue = false;

	for (int i = 1; i < argc; i++)
	{
		const std::string_view arg = argv[i];

		if (parseOptions)
		{
			switch (ParseAndPushOption(arg))
			{
				case OptionType::NOT_AN_OPTION:
					if (attachValue)
					{
						// m_options cannot be empty if attachValue is true
						m_options.back().value = arg;
						attachValue = false;
					}
					else
					{
						PushOperand(arg);
					}
					break;
				case OptionType::LONG_OPTION:
				case OptionType::SHORT_OPTION:
					attachValue = true;
					break;
				case OptionType::LONG_OPTION_WITH_VALUE:
					attachValue = false;
					break;
				case OptionType::END_OF_OPTIONS:
					parseOptions = false;
					attachValue = false;
					break;
			}
		}
		else
		{
			PushOperand(arg);
		}
	}
}

CmdLine::OptionType CmdLine::ParseAndPushOption(const std::string_view& arg)
{
	if (arg.length() < 2 || arg[0] != '-')
	{
		return OptionType::NOT_AN_OPTION;
	}

	if (arg[1] == '-')
	{
		if (arg.length() == 2)
		{
			// found "--"
			return OptionType::END_OF_OPTIONS;
		}
		else
		{
			// long option
			std::string_view name = arg;

			// remove "--"
			name.remove_prefix(2);

			const auto equalSignPos = name.find('=');
			if (equalSignPos != std::string_view::npos)
			{
				// "option=value"
				std::string_view value = name;

				// remove "option="
				value.remove_prefix(equalSignPos + 1);
				// remove "=value"
				name.remove_suffix(name.length() - equalSignPos);

				PushOption(name, value);

				return OptionType::LONG_OPTION_WITH_VALUE;
			}
			else
			{
				PushOption(name);

				return OptionType::LONG_OPTION;
			}
		}
	}
	else
	{
		// short option
		std::string_view name = arg;

		// remove "-"
		name.remove_prefix(1);

		if (name.length() > 1)
		{
			// multiple short options packed together
			for (unsigned int i = 0; i < name.length(); i++)
			{
				PushOption(name.substr(i, 1));
			}
		}
		else
		{
			PushOption(name);
		}

		return OptionType::SHORT_OPTION;
	}
}

void CmdLine::PushOption(const std::string_view& name, const std::string_view& value)
{
	Option option;
	option.name = name;
	option.value = value;

	m_options.emplace_back(option);
}

void CmdLine::PushOperand(const std::string_view& operand)
{
	m_operands.emplace_back(operand);
}

std::string_view CmdLine::PopRequiredOptionWithRequiredValue(const std::string_view& name)
{
	std::string_view value;
	if (!TryPopOptionWithRequiredValue(name, value))
		throw std::runtime_error("Option '" + PrettifyOption(name) + "' is missing");

	return value;
}

std::string_view CmdLine::PopRequiredOptionWithRequiredValue(const std::string_view& name, const std::string_view& name2)
{
	std::string_view value;
	if (!TryPopOptionWithRequiredValue(name, name2, value))
		throw std::runtime_error("Option '" + PrettifyOption(name, name2) + "' is missing");

	return value;
}

std::string_view CmdLine::PopRequiredOptionWithOptionalValue(const std::string_view& name)
{
	std::string_view value;
	if (!TryPopOptionWithOptionalValue(name, value))
		throw std::runtime_error("Option '" + PrettifyOption(name) + "' is missing");

	return value;
}

std::string_view CmdLine::PopRequiredOptionWithOptionalValue(const std::string_view& name, const std::string_view& name2)
{
	std::string_view value;
	if (!TryPopOptionWithOptionalValue(name, name2, value))
		throw std::runtime_error("Option '" + PrettifyOption(name, name2) + "' is missing");

	return value;
}

bool CmdLine::TryPopOptionWithoutValue(const std::string_view& name)
{
	std::string_view value;
	if (!TryPopOptionWithOptionalValue(name, value))
		return false;

	// so this value is actually an operand
	if (!value.empty())
		PushOperand(value);

	return true;
}

bool CmdLine::TryPopOptionWithoutValue(const std::string_view& name, const std::string_view& name2)
{
	std::string_view value;
	if (!TryPopOptionWithOptionalValue(name, name2, value))
		return false;

	// so this value is actually an operand
	if (!value.empty())
		PushOperand(value);

	return true;
}

bool CmdLine::TryPopOptionWithRequiredValue(const std::string_view& name, std::string_view& value)
{
	if (!TryPopOptionWithOptionalValue(name, value))
		return false;

	if (value.empty())
		throw std::runtime_error("Option '" + PrettifyOption(name) + "' requires a value");

	return true;
}

bool CmdLine::TryPopOptionWithRequiredValue(const std::string_view& name, const std::string_view& name2, std::string_view& value)
{
	if (!TryPopOptionWithOptionalValue(name, name2, value))
		return false;

	if (value.empty())
		throw std::runtime_error("Option '" + PrettifyOption(name, name2) + "' requires a value");

	return true;
}

bool CmdLine::TryPopOptionWithOptionalValue(const std::string_view& name, std::string_view& value)
{
	// use reverse iterators to get the last matching option from the command line
	const auto it = std::find(m_options.rbegin(), m_options.rend(), name);
	if (it == m_options.rend())
		return false;

	value = it->value;

	// pop the option
	m_options.erase(std::next(it).base());

	return true;
}

bool CmdLine::TryPopOptionWithOptionalValue(const std::string_view& name, const std::string_view& name2, std::string_view& value)
{
	if (name < name2)
		return TryPopOptionWithOptionalValue(name2, value) || TryPopOptionWithOptionalValue(name, value);
	else
		return TryPopOptionWithOptionalValue(name, value) || TryPopOptionWithOptionalValue(name2, value);
}

unsigned int CmdLine::CountAndPopOption(const std::string_view& name)
{
	const auto originalSize = m_options.size();

	m_options.erase(std::remove(m_options.begin(), m_options.end(), name), m_options.end());

	return originalSize - m_options.size();
}

unsigned int CmdLine::CountAndPopOption(const std::string_view& name, const std::string_view& name2)
{
	return CountAndPopOption(name) + CountAndPopOption(name2);
}

void CmdLine::Clear()
{
	m_options.clear();
	m_operands.clear();
}

std::string CmdLine::PrettifyOption(const std::string_view& name)
{
	if (name.length() == 1)
		return "-" + std::string(name);
	else if (name.length() > 1)
		return "--" + std::string(name);
	else
		return {};
}

std::string CmdLine::PrettifyOption(const std::string_view& name, const std::string_view& name2)
{
	return PrettifyOption(name) + "|" + PrettifyOption(name2);
}
