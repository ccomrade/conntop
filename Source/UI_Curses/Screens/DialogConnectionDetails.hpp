/**
 * @file
 * @brief DialogConnectionDetails class.
 */

#pragma once

#include <string>

#include "Screen.hpp"
#include "Connection.hpp"

namespace ctp
{
	class ScreenConnectionList;

	class DialogConnectionDetails : public Screen
	{
		enum EContent
		{
			NONE,
			NORMAL,
			REMOVED
		};

		EContent m_content;
		bool m_hasPorts;

		// static entries
		std::string m_typeName;
		std::string m_srcAddress;
		std::string m_dstAddress;
		std::string m_srcPort;
		std::string m_dstPort;

		// dynamic entries
		std::string m_stateName;
		std::string m_srcHostname;
		std::string m_dstHostname;
		std::string m_srcService;
		std::string m_dstService;
		Country m_srcCountry;
		Country m_dstCountry;
		ASN m_srcASN;
		ASN m_dstASN;
		ConnectionTraffic m_traffic;

		void drawStatic();
		void drawUpdate(int updateFlags);
		void writeStringSafe(const KString & string);
		void fillEmpty(int count);
		void fillEmpty();

		void handleResize() override;
		bool handleKey(int ch) override;

	public:
		DialogConnectionDetails(ScreenConnectionList *parent);

		void open(const ConnectionData *data);
		void update(const ConnectionData *data, int updateFlags);
		void close();
	};
}
