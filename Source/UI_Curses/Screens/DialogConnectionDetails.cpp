/**
 * @file
 * @brief Implementation of DialogConnectionDetails class.
 */

#include "DialogConnectionDetails.hpp"
#include "ScreenConnectionList.hpp"
#include "ColorSystem.hpp"
#include "Util.hpp"  // Util::GetHumanReadableSize

namespace ctp
{
	static std::string PortToString( const PortData & port )
	{
		std::string result;

		result += port.getPortTypeName();
		result += ' ';
		result += port.getNumericString();

		return result;
	}

	DialogConnectionDetails::DialogConnectionDetails( ScreenConnectionList *parent )
	: Screen({ 80, 22 }, { 80, 22 }, parent),
	  m_content(NONE),
	  m_hasPorts(),
	  m_typeName(),
	  m_srcAddress(),
	  m_dstAddress(),
	  m_srcPort(),
	  m_dstPort(),
	  m_stateName(),
	  m_srcHostname(),
	  m_dstHostname(),
	  m_srcService(),
	  m_dstService(),
	  m_srcCountry(),
	  m_dstCountry(),
	  m_srcASN(),
	  m_dstASN(),
	  m_traffic()
	{
		drawStatic();
	}

	void DialogConnectionDetails::open( const ConnectionData *data )
	{
		ScreenConnectionList *parent = static_cast<ScreenConnectionList*>( getParentScreen() );

		parent->pushDialog( this );

		if ( data )
		{
			m_content = NORMAL;

			m_hasPorts = data->hasPorts();
			m_typeName = data->getTypeName();
			m_srcAddress = data->getSrcAddr().getNumericString();
			m_dstAddress = data->getDstAddr().getNumericString();

			if ( m_hasPorts )
			{
				m_srcPort = PortToString( data->getSrcPort() );
				m_dstPort = PortToString( data->getDstPort() );
			}
		}

		drawStatic();

		if ( m_content == NORMAL )
		{
			update( data, -1 );
		}
	}

	void DialogConnectionDetails::update( const ConnectionData *data, int updateFlags )
	{
		if ( m_content == NONE )
		{
			return;
		}

		if ( ! data )
		{
			m_content = REMOVED;
			drawUpdate( EConnectionUpdateFlags::PROTO_STATE );

			return;
		}

		int flags = 0;

		if ( updateFlags & EConnectionUpdateFlags::PROTO_STATE )
		{
			if ( m_stateName != data->getStateName() )
			{
				m_stateName = data->getStateName();
				flags |= EConnectionUpdateFlags::PROTO_STATE;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::SRC_ADDRESS )
		{
			if ( m_srcHostname != data->getSrcAddr().getHostnameString() )
			{
				m_srcHostname = data->getSrcAddr().getHostnameString();
				flags |= EConnectionUpdateFlags::SRC_ADDRESS;
			}

			if ( m_srcCountry != data->getSrcAddr().getCountry() )
			{
				m_srcCountry = data->getSrcAddr().getCountry();
				flags |= EConnectionUpdateFlags::SRC_ADDRESS;
			}

			if ( m_srcASN != data->getSrcAddr().getASN() )
			{
				m_srcASN = data->getSrcAddr().getASN();
				flags |= EConnectionUpdateFlags::SRC_ADDRESS;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::DST_ADDRESS )
		{
			if ( m_dstHostname != data->getDstAddr().getHostnameString() )
			{
				m_dstHostname = data->getDstAddr().getHostnameString();
				flags |= EConnectionUpdateFlags::DST_ADDRESS;
			}

			if ( m_dstCountry != data->getDstAddr().getCountry() )
			{
				m_dstCountry = data->getDstAddr().getCountry();
				flags |= EConnectionUpdateFlags::DST_ADDRESS;
			}

			if ( m_dstASN != data->getDstAddr().getASN() )
			{
				m_dstASN = data->getDstAddr().getASN();
				flags |= EConnectionUpdateFlags::DST_ADDRESS;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::SRC_PORT )
		{
			if ( m_srcService != data->getSrcPort().getServiceString() )
			{
				m_srcService = data->getSrcPort().getServiceString();
				flags |= EConnectionUpdateFlags::SRC_PORT;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::DST_PORT )
		{
			if ( m_dstService != data->getDstPort().getServiceString() )
			{
				m_dstService = data->getDstPort().getServiceString();
				flags |= EConnectionUpdateFlags::DST_PORT;
			}
		}

		const ConnectionTraffic & traffic = data->getTraffic();

		if ( updateFlags & EConnectionUpdateFlags::RX_PACKETS )
		{
			if ( m_traffic.rxPackets != traffic.rxPackets )
			{
				m_traffic.rxPackets = traffic.rxPackets;
				flags |= EConnectionUpdateFlags::RX_PACKETS;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::TX_PACKETS )
		{
			if ( m_traffic.txPackets != traffic.txPackets )
			{
				m_traffic.txPackets = traffic.txPackets;
				flags |= EConnectionUpdateFlags::TX_PACKETS;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::RX_BYTES )
		{
			if ( m_traffic.rxBytes != traffic.rxBytes )
			{
				m_traffic.rxBytes = traffic.rxBytes;
				flags |= EConnectionUpdateFlags::RX_BYTES;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::TX_BYTES )
		{
			if ( m_traffic.txBytes != traffic.txBytes )
			{
				m_traffic.txBytes = traffic.txBytes;
				flags |= EConnectionUpdateFlags::TX_BYTES;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::RX_SPEED )
		{
			if ( m_traffic.rxSpeed != traffic.rxSpeed )
			{
				m_traffic.rxSpeed = traffic.rxSpeed;
				flags |= EConnectionUpdateFlags::RX_SPEED;
			}
		}

		if ( updateFlags & EConnectionUpdateFlags::TX_SPEED )
		{
			if ( m_traffic.txSpeed != traffic.txSpeed )
			{
				m_traffic.txSpeed = traffic.txSpeed;
				flags |= EConnectionUpdateFlags::TX_SPEED;
			}
		}

		drawUpdate( flags );
	}

	void DialogConnectionDetails::close()
	{
		ScreenConnectionList *parent = static_cast<ScreenConnectionList*>( getParentScreen() );

		parent->removeDialog( this );

		m_content = NONE;
		m_hasPorts = false;
		m_typeName.clear();
		m_srcAddress.clear();
		m_dstAddress.clear();
		m_srcPort.clear();
		m_dstPort.clear();
		m_stateName.clear();
		m_srcHostname.clear();
		m_dstHostname.clear();
		m_srcService.clear();
		m_dstService.clear();
		m_srcCountry = Country();
		m_dstCountry = Country();
		m_srcASN = ASN();
		m_dstASN = ASN();
		m_traffic = ConnectionTraffic();

		// clear dialog window
		for ( int i = 0; i < getHeight(); i++ )
		{
			setPos( 1, i );
			fillEmpty();
		}
	}

	void DialogConnectionDetails::drawStatic()
	{
		box( getWindow(), 0, 0 );

		if ( m_content == NONE )
		{
			return;
		}

		const int labelAttr = gColorSystem->getAttr( ColorSystem::ATTR_STATUS_LABEL );
		const int valueAttr = gColorSystem->getAttr( ColorSystem::ATTR_STATUS_VALUE );

		// labels begin

		enableAttr( labelAttr );

		// - header

		setPos( 1, 1 );
		writeString( "Type: " );

		setPos( 1, 2 );
		writeString( "State: " );

		// - source

		setPos( 1, 4 );
		writeString( "Source: " );

		setPos( 1, 5 );
		writeString( "  Address: " );

		setPos( 1, 6 );
		writeString( "  Hostname: " );

		setPos( 1, 7 );
		writeString( "  Organization: " );

		setPos( 1, 8 );
		writeString( "  Country: " );

		if ( m_hasPorts )
		{
			setPos( 1, 9 );
			writeString( "  Port: " );
		}

		// - destination

		setPos( 1, 11 );
		writeString( "Destination: " );

		setPos( 1, 12 );
		writeString( "  Address: " );

		setPos( 1, 13 );
		writeString( "  Hostname: " );

		setPos( 1, 14 );
		writeString( "  Organization: " );

		setPos( 1, 15 );
		writeString( "  Country: " );

		if ( m_hasPorts )
		{
			setPos( 1, 16 );
			writeString( "  Port: " );
		}

		// - traffic

		setPos( 1, 18 );
		writeString( "Traffic: " );

		setPos( 1, 19 );
		writeString( "  Received: " );

		setPos( 1, 20 );
		writeString( "  Sent:     " );

		setPos( 36, 19 );
		writeString( " | " );

		setPos( 36, 20 );
		writeString( " | " );

		disableAttr( labelAttr );

		// labels end

		// values begin

		enableAttr( valueAttr );

		// - header

		setPos( 7, 1 );
		writeString( m_typeName );

		// - source

		setPos( 12, 5 );
		writeString( m_srcAddress );

		if ( m_hasPorts )
		{
			setPos( 9, 9 );
			writeString( m_srcPort );
		}

		// - destination

		setPos( 12, 12 );
		writeString( m_dstAddress );

		if ( m_hasPorts )
		{
			setPos( 9, 16 );
			writeString( m_dstPort );
		}

		// - traffic

		setPos( 13, 19 );
		writeString( "      0 B" );

		setPos( 13, 20 );
		writeString( "      0 B" );

		setPos( 39, 19 );
		writeString( "0 packets" );
		fillEmpty();

		setPos( 39, 20 );
		writeString( "0 packets" );
		fillEmpty();

		disableAttr( valueAttr );

		// values end
	}

	void DialogConnectionDetails::drawUpdate( int updateFlags )
	{
		const int valueAttr = gColorSystem->getAttr( ColorSystem::ATTR_STATUS_VALUE );

		enableAttr( valueAttr );

		// header

		if ( updateFlags & EConnectionUpdateFlags::PROTO_STATE )
		{
			setPos( 8, 2 );
			if ( m_content == REMOVED )
			{
				const int attr = gColorSystem->getAttr( ColorSystem::ATTR_IMPORTANT_RED );

				disableAttr( valueAttr );
				enableAttr( attr );

				writeString( "<removed>" );

				disableAttr( attr );
				enableAttr( valueAttr );
			}
			else if ( ! m_stateName.empty() )
			{
				writeString( m_stateName );
			}
			fillEmpty();
		}

		// source

		if ( updateFlags & EConnectionUpdateFlags::SRC_ADDRESS )
		{
			setPos( 13, 6 );
			if ( ! m_srcHostname.empty() )
			{
				writeStringSafe( m_srcHostname );
			}
			fillEmpty();

			setPos( 17, 7 );
			if ( ! m_srcASN.isEmpty() )
			{
				writeString( m_srcASN.getString() );
				writeChar( ' ' );
				writeStringSafe( m_srcASN.getOrgName() );
			}
			fillEmpty();

			setPos( 12, 8 );
			if ( ! m_srcCountry.isUnknown() )
			{
				writeString( m_srcCountry.getCodeString() );
				writeChar( ' ' );
				writeChar( '(' );
				writeString( m_srcCountry.getNameString() );
				writeChar( ')' );
			}
			fillEmpty();
		}

		if ( updateFlags & EConnectionUpdateFlags::SRC_PORT && m_hasPorts )
		{
			setPos( 9 + m_srcPort.length(), 9 );
			if ( ! m_srcService.empty() )
			{
				writeChar( ' ' );
				writeChar( '(' );
				writeStringSafe( m_srcService );
				writeChar( ')' );
			}
			fillEmpty();
		}

		// destination

		if ( updateFlags & EConnectionUpdateFlags::DST_ADDRESS )
		{
			setPos( 13, 13 );
			if ( ! m_dstHostname.empty() )
			{
				writeStringSafe( m_dstHostname );
			}
			fillEmpty();

			setPos( 17, 14 );
			if ( ! m_dstASN.isEmpty() )
			{
				writeString( m_dstASN.getString() );
				writeChar( ' ' );
				writeStringSafe( m_dstASN.getOrgName() );
			}
			fillEmpty();

			setPos( 12, 15 );
			if ( ! m_dstCountry.isUnknown() )
			{
				writeString( m_dstCountry.getCodeString() );
				writeChar( ' ' );
				writeChar( '(' );
				writeString( m_dstCountry.getNameString() );
				writeChar( ')' );
			}
			fillEmpty();
		}

		if ( updateFlags & EConnectionUpdateFlags::DST_PORT && m_hasPorts )
		{
			setPos( 9 + m_dstPort.length(), 16 );
			if ( ! m_dstService.empty() )
			{
				writeChar( ' ' );
				writeChar( '(' );
				writeStringSafe( m_dstService );
				writeChar( ')' );
			}
			fillEmpty();
		}

		// traffic

		if ( updateFlags & EConnectionUpdateFlags::RX_PACKETS )
		{
			setPos( 39, 19 );
			writeString( std::to_string ( m_traffic.rxPackets ) );
			writeString( (m_traffic.rxPackets == 1) ? " packet" : " packets" );
			fillEmpty();
		}

		if ( updateFlags & EConnectionUpdateFlags::TX_PACKETS )
		{
			setPos( 39, 20 );
			writeString( std::to_string ( m_traffic.txPackets ) );
			writeString( (m_traffic.txPackets == 1) ? " packet" : " packets" );
			fillEmpty();
		}

		if ( updateFlags & EConnectionUpdateFlags::RX_BYTES )
		{
			setPos( 13, 19 );
			std::string value = Util::GetHumanReadableSize( m_traffic.rxBytes );
			fillEmpty( 9 - value.length() );
			writeString( value );
		}

		if ( updateFlags & EConnectionUpdateFlags::TX_BYTES )
		{
			setPos( 13, 20 );
			std::string value = Util::GetHumanReadableSize( m_traffic.txBytes );
			fillEmpty( 9 - value.length() );
			writeString( value );
		}

		if ( updateFlags & EConnectionUpdateFlags::RX_SPEED )
		{
			setPos( 22, 19 );
			if ( m_traffic.rxSpeed > 0 )
			{
				writeString( " (" );
				writeString( Util::GetHumanReadableSize( m_traffic.rxSpeed ) );
				writeString( "/s)" );
			}
			fillEmpty( 36 - getPos().x );
		}

		if ( updateFlags & EConnectionUpdateFlags::TX_SPEED )
		{
			setPos( 22, 20 );
			if ( m_traffic.txSpeed > 0 )
			{
				writeString( " (" );
				writeString( Util::GetHumanReadableSize( m_traffic.txSpeed ) );
				writeString( "/s)" );
			}
			fillEmpty( 36 - getPos().x );
		}

		// end

		disableAttr( valueAttr );
	}

	void DialogConnectionDetails::writeStringSafe( const KString & string )
	{
		unsigned int space = getWidth() - getPos().x - 1;

		if ( string.length() <= space )
		{
			writeString( string );
		}
		else
		{
			if ( space > 2 )
			{
				writeString( string, space-2 );
			}

			writeString( ".." );
		}
	}

	void DialogConnectionDetails::fillEmpty( int count )
	{
		while ( count > 0 )
		{
			writeChar( ' ' );
			count--;
		}
	}

	void DialogConnectionDetails::fillEmpty()
	{
		fillEmpty( getWidth() - getPos().x - 1 );  // window box
	}

	void DialogConnectionDetails::handleResize()
	{
		drawStatic();
		drawUpdate( -1 );
	}

	bool DialogConnectionDetails::handleKey( int ch )
	{
		switch ( ch )
		{
			case KEY_UP:
			case KEY_DOWN:
			case KEY_LEFT:
			case KEY_RIGHT:
			case KEY_PPAGE:
			case KEY_NPAGE:
			{
				return true;
			}
			case '\015':  // ENTER key
			case '\033':  // ESC key
			{
				close();

				return true;
			}
		}

		return false;
	}
}
