/**
 * @file
 * @brief Implementation of ScreenHelp class.
 */

#include "ScreenHelp.hpp"
#include "App.hpp"
#include "CursesEvent.hpp"
#include "ColorSystem.hpp"
#include "Version.hpp"

namespace ctp
{
	ScreenHelp::ScreenHelp()
	: Screen({ 80, 27 }, { 80, 27 })
	{
		draw();
	}

	void ScreenHelp::draw()
	{
		const int labelAttr = gColorSystem->getAttr( ColorSystem::ATTR_STATUS_VALUE );
		const int titleAttr = gColorSystem->getAttr( ColorSystem::ATTR_IMPORTANT_GREEN );

		box( getWindow(), 0, 0 );

		setPos( 1, 1 );
		writeString( VERSION );
		setPos( 1, 2 );
		writeString( COPYRIGHT_NOTICE );

		// controls

		setPos( 1, 4 );
		enableAttr( titleAttr );
		writeString( "Controls:" );
		disableAttr( titleAttr );

		setPos( 1, 5 );
		enableAttr( labelAttr );
		writeString( "  Up/Down or PageUp/PageDown: " );
		disableAttr( labelAttr );
		writeString( "Move cursor and/or scroll content vertically" );

		setPos( 1, 6 );
		enableAttr( labelAttr );
		writeString( "  Left/Right: " );
		disableAttr( labelAttr );
		writeString( "Scroll content horizontally if the screen is not large enough" );

		setPos( 1, 7 );
		enableAttr( labelAttr );
		writeString( "  Enter: " );
		disableAttr( labelAttr );
		writeString( "Open details dialog of current item (only if cursor is enabled)" );

		setPos( 1, 8 );
		enableAttr( labelAttr );
		writeString( "  Esc: " );
		disableAttr( labelAttr );
		writeString( "Close any dialog window" );

		setPos( 1, 9 );
		enableAttr( labelAttr );
		writeString( "  F1: " );
		disableAttr( labelAttr );
		writeString( "Show/Hide this help" );

		setPos( 1, 10 );
		enableAttr( labelAttr );
		writeString( "  F2: " );
		disableAttr( labelAttr );
		writeString( "Select columns and change their width" );

		setPos( 1, 11 );
		enableAttr( labelAttr );
		writeString( "  F3: " );
		disableAttr( labelAttr );
		writeString( "Change sort mode" );

		setPos( 1, 12 );
		enableAttr( labelAttr );
		writeString( "  F4: " );
		disableAttr( labelAttr );
		writeString( "Show/Hide cursor" );

		setPos( 1, 13 );
		enableAttr( labelAttr );
		writeString( "  F5: " );
		disableAttr( labelAttr );
		writeString( "Toggle display of address hostname" );

		setPos( 1, 14 );
		enableAttr( labelAttr );
		writeString( "  F6: " );
		disableAttr( labelAttr );
		writeString( "Toggle display of port service name" );

		setPos( 1, 15 );
		enableAttr( labelAttr );
		writeString( "  F7: " );
		disableAttr( labelAttr );
		writeString( "Previous mode (if any)" );

		setPos( 1, 16 );
		enableAttr( labelAttr );
		writeString( "  F8: " );
		disableAttr( labelAttr );
		writeString( "Next mode (if any)" );

		setPos( 1, 17 );
		enableAttr( labelAttr );
		writeString( "  F9 or Space: " );
		disableAttr( labelAttr );
		writeString( "Toggle update pause" );

		setPos( 1, 18 );
		enableAttr( labelAttr );
		writeString( "  F10 or Control-C: " );
		disableAttr( labelAttr );
		writeString( "Quit the application" );

		// dialog controls

		setPos( 1, 20 );
		enableAttr( titleAttr );
		writeString( "Dialog Controls:" );
		disableAttr( titleAttr );

		setPos( 1, 21 );
		enableAttr( labelAttr );
		writeString( "  Up/Down or PageUp/PageDown: " );
		disableAttr( labelAttr );
		writeString( "Move cursor (if any)" );

		setPos( 1, 22 );
		enableAttr( labelAttr );
		writeString( "  Left/Right: " );
		disableAttr( labelAttr );
		writeString( "Change value of current item (if any)" );

		setPos( 1, 23 );
		enableAttr( labelAttr );
		writeString( "  Space: " );
		disableAttr( labelAttr );
		writeString( "Toggle current check box" );

		setPos( 1, 24 );
		enableAttr( labelAttr );
		writeString( "  Enter: " );
		disableAttr( labelAttr );
		writeString( "Close the dialog and apply changes" );

		setPos( 1, 25 );
		enableAttr( labelAttr );
		writeString( "  Esc: " );
		disableAttr( labelAttr );
		writeString( "Close the dialog without saving changes" );
	}

	void ScreenHelp::handleResize()
	{
		draw();
	}

	bool ScreenHelp::handleKey( int ch )
	{
		switch ( ch )
		{
			case '\033':  // ESC key
			case KEY_F(1):
			{
				gApp->getEventSystem()->dispatch<CursesEvent>( CursesEvent::CLOSE_HELP );
				return true;
			}
		}

		return false;
	}
}
