/**
 * @file
 * @brief CmdLine class.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <exception>

#include "GlobalEnvironment.hpp"
#include "KString.hpp"

namespace ctp
{
	enum struct ECmdLineArgValue
	{
		NONE,
		OPTIONAL,
		REQUIRED
	};

	class CmdLineArgConfig
	{
		KString m_shortName;
		KString m_description;
		ECmdLineArgValue m_valueType;
		KString m_valueTypeName;

	public:
		constexpr CmdLineArgConfig( const char *shortName, const char *description,
		                            ECmdLineArgValue valueType = ECmdLineArgValue::NONE, const char *valueTypeName = "" )
		: m_shortName(shortName),
		  m_description(description),
		  m_valueType(valueType),
		  m_valueTypeName(valueTypeName)
		{
		}

		constexpr ECmdLineArgValue getValueType() const
		{
			return m_valueType;
		}

		constexpr bool hasShortName() const
		{
			return ! m_shortName.empty();
		}

		constexpr bool hasDescription() const
		{
			return ! m_description.empty();
		}

		constexpr bool hasValueTypeName() const
		{
			return ! m_valueTypeName.empty();
		}

		constexpr KString getShortName() const
		{
			return m_shortName;
		}

		constexpr KString getDescription() const
		{
			return m_description;
		}

		constexpr KString getValueTypeName() const
		{
			return m_valueTypeName;
		}
	};

	class CmdLineArg
	{
		const std::pair<const KString, CmdLineArgConfig> *m_config;
		std::vector<KString> m_values;
		unsigned int m_count;

		friend class CmdLine;

	public:
		CmdLineArg( const std::pair<const KString, CmdLineArgConfig> & config )
		: m_config(&config),
		  m_values(),
		  m_count()
		{
		}

		bool hasValue() const
		{
			return ! m_values.empty();
		}

		bool hasShortName() const
		{
			return m_config->second.hasShortName();
		}

		bool hasDescription() const
		{
			return m_config->second.hasDescription();
		}

		bool hasValueTypeName() const
		{
			return m_config->second.hasValueTypeName();
		}

		bool canHaveValue() const
		{
			return m_config->second.getValueType() != ECmdLineArgValue::NONE;
		}

		bool requiresValue() const
		{
			return m_config->second.getValueType() == ECmdLineArgValue::REQUIRED;
		}

		KString getValue() const
		{
			return (m_values.empty()) ? KString() : m_values.back();
		}

		const std::vector<KString> & getAllValues() const
		{
			return m_values;
		}

		KString getName() const
		{
			return m_config->first;
		}

		KString getShortName() const
		{
			return m_config->second.getShortName();
		}

		KString getDescription() const
		{
			return m_config->second.getDescription();
		}

		KString getValueTypeName() const
		{
			return m_config->second.getValueTypeName();
		}

		unsigned int getCount() const
		{
			return m_count;
		}

		const CmdLineArgConfig & getConfig() const
		{
			return m_config->second;
		}
	};

	class CmdLineParseException : public std::exception
	{
		std::string m_what;

	public:
		CmdLineParseException( std::string what )
		: m_what(std::move( what ))
		{
		}

		const char *what() const noexcept override
		{
			return m_what.c_str();
		}

		const std::string & getString() const
		{
			return m_what;
		}
	};

	class CmdLine
	{
		std::map<KString, CmdLineArg> m_args;
		std::vector<KString> m_nonOptionArgs;

		void parse( int argc, char *argv[] );

	public:
		CmdLine( int argc, char *argv[] )
		: m_args(),
		  m_nonOptionArgs()
		{
			parse( argc, argv );  // may throw CmdLineParseException
		}

		CmdLineArg *getArg( const KString & arg ) const
		{
			auto it = m_args.find( arg );
			return (it != m_args.end()) ? const_cast<CmdLineArg*>( &it->second ) : nullptr;
		}

		bool hasArg( const KString & arg ) const
		{
			return m_args.find( arg ) != m_args.end();
		}

		const std::vector<KString> & getNonOptionArgs() const
		{
			return m_nonOptionArgs;
		}

		unsigned int getArgCount() const
		{
			return m_args.size();
		}

		static std::string CreateOptionsList();

		static const std::map<KString, CmdLineArgConfig> OPTIONS;
	};
}
