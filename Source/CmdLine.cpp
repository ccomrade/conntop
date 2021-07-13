/**
 * @file
 * @brief Implementation of CmdLine class.
 */

#include <sstream>
#include <iomanip>

#include "CmdLine.hpp"

static CmdLineArg *AddShortArg(char argChar, std::map<KString, CmdLineArg> & argMap)
{
	for (auto it = argMap.begin(); it != argMap.end(); ++it)
	{
		CmdLineArg & arg = it->second;

		if (arg.hasShortName() && arg.getShortName()[0] == argChar)
		{
			return &arg;
		}
	}

	for (auto optionIt = CmdLine::OPTIONS.begin(); optionIt != CmdLine::OPTIONS.end(); ++optionIt)
	{
		const KString & optionName = optionIt->first;
		const CmdLineArgConfig & optionConfig = optionIt->second;

		if (optionConfig.hasShortName() && optionConfig.getShortName()[0] == argChar)
		{
			auto result = argMap.emplace(optionName, *optionIt);
			auto argIt = result.first;
			return &argIt->second;
		}
	}

	return nullptr;
}

static CmdLineArg *AddLongArg(const KString & argName, std::map<KString, CmdLineArg> & argMap)
{
	auto it = argMap.find(argName);
	if (it != argMap.end())
	{
		return &it->second;
	}

	auto optionIt = CmdLine::OPTIONS.find(argName);
	if (optionIt != CmdLine::OPTIONS.end())
	{
		const KString & optionName = optionIt->first;

		auto result = argMap.emplace(optionName, *optionIt);
		auto argIt = result.first;
		return &argIt->second;
	}

	return nullptr;
}

void CmdLine::parse(int argc, char *argv[])
{
	if (argc < 1 || argv == nullptr)
	{
		return;
	}

	const KString appName = argv[0];

	int i = 1;

	CmdLineArg *waitingArg = nullptr;
	bool isWaitingArgShort = false;

	for (; i < argc; i++)
	{
		const KString arg = argv[i];

		if (arg.length() > 1 && arg[0] == '-')
		{
			if (waitingArg && waitingArg->requiresValue())
			{
				break;
			}

			if (arg[1] != '-')
			{
				isWaitingArgShort = true;

				for (size_t j = 1; j < arg.length(); j++)
				{
					char argChar = arg[j];

					waitingArg = AddShortArg(argChar, m_args);
					if (!waitingArg)
					{
						std::string msg = appName;
						msg += ": invalid option '-";
						msg += argChar;
						msg += "'";
						throw CmdLineParseException(std::move(msg));
					}

					waitingArg->m_count++;

					if (waitingArg->canHaveValue())
					{
						const size_t pos = j+1;
						if (pos < arg.length())
						{
							waitingArg->m_values.emplace_back(arg.c_str() + pos);
							waitingArg = nullptr;
						}
						break;
					}
				}
			}
			else
			{
				if (arg == "--")
				{
					i++;
					break;
				}

				isWaitingArgShort = false;

				size_t valueSignPos = 2;  // skip "--"
				for (; valueSignPos < arg.length(); valueSignPos++)
				{
					if (arg[valueSignPos] == '=')
					{
						break;
					}
				}

				const char *argValue = nullptr;
				if (valueSignPos < arg.length())
				{
					argValue = arg.c_str() + valueSignPos + 1;  // skip "="
					std::string argName(arg.c_str() + 2, valueSignPos - 2);  // skip "--"
					waitingArg = AddLongArg(argName, m_args);
				}
				else
				{
					waitingArg = AddLongArg(arg.c_str() + 2, m_args);  // skip "--"
				}

				if (!waitingArg)
				{
					std::string msg = appName;
					msg += ": unrecognized option '";
					msg += arg;
					msg += "'";
					throw CmdLineParseException(std::move(msg));
				}

				waitingArg->m_count++;

				if (argValue)
				{
					if (waitingArg->canHaveValue())
					{
						waitingArg->m_values.emplace_back(argValue);
						waitingArg = nullptr;
					}
					else
					{
						std::string msg = appName;
						msg += ": option '--";
						msg += waitingArg->getName();
						msg += "' doesn't allow an argument";
						throw CmdLineParseException(std::move(msg));
					}
				}
			}
		}
		else if (waitingArg && waitingArg->canHaveValue())
		{
			waitingArg->m_values.emplace_back(arg);
			waitingArg = nullptr;
		}
		else
		{
			m_nonOptionArgs.emplace_back(arg);
		}
	}

	if (waitingArg && waitingArg->requiresValue())
	{
		std::string msg = appName;
		msg += ": option '";
		msg += (isWaitingArgShort) ? "-" : "--";
		msg += (isWaitingArgShort) ? waitingArg->getShortName() : waitingArg->getName();
		msg += "' requires an argument";
		throw CmdLineParseException(std::move(msg));
	}

	for (; i < argc; i++)
	{
		m_nonOptionArgs.emplace_back(argv[i]);
	}
}

std::string CmdLine::CreateOptionsList()
{
	unsigned int maxNameLength = 4;
	for (auto it = OPTIONS.begin(); it != OPTIONS.end(); ++it)
	{
		const KString & optionName = it->first;
		const CmdLineArgConfig & optionConfig = it->second;

		unsigned int length = optionName.length() + optionConfig.getValueTypeName().length();

		switch (optionConfig.getValueType())
		{
			case ECmdLineArgValue::NONE:
			{
				break;
			}
			case ECmdLineArgValue::OPTIONAL:
			{
				length += 3;  // "=[]"
				break;
			}
			case ECmdLineArgValue::REQUIRED:
			{
				length += 1;  // "="
				break;
			}
		}

		if (length > maxNameLength)
		{
			maxNameLength = length;
		}
	}

	std::ostringstream buffer;
	for (auto it = OPTIONS.begin(); it != OPTIONS.end(); ++it)
	{
		const KString & optionName = it->first;
		const CmdLineArgConfig & optionConfig = it->second;

		if (optionConfig.hasShortName())
		{
			buffer << " -" << optionConfig.getShortName();
		}
		else
		{
			buffer << "   ";
		}

		buffer << " --" << std::setw(maxNameLength) << std::left;
		switch (optionConfig.getValueType())
		{
			case ECmdLineArgValue::NONE:
			{
				buffer << optionName;
				break;
			}
			case ECmdLineArgValue::OPTIONAL:
			{
				std::string name = optionName;
				name += '=';
				name += '[';
				name += optionConfig.getValueTypeName();
				name += ']';
				buffer << name;
				break;
			}
			case ECmdLineArgValue::REQUIRED:
			{
				std::string name = optionName;
				name += '=';
				name += optionConfig.getValueTypeName();
				buffer << name;
				break;
			}
		}

		if (optionConfig.hasDescription())
		{
			buffer << "  " << optionConfig.getDescription();
		}

		buffer << std::endl;
	}

	return buffer.str();
}
