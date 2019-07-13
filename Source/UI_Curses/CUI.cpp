/**
 * @file
 * @brief Implementation of Curses UI.
 */

#include "CUI.hpp"
#include "Curses.hpp"

namespace ctp
{
	class CUI_Curses::Impl
	{
		Curses m_curses;

	public:
		Impl()
		: m_curses()
		{
		}

		void init()
		{
			m_curses.init();
		}

		void refreshConnectionList()
		{
			m_curses.getConnectionListScreen().updateList();
			m_curses.refreshScreen();
		}
	};

	CUI_Curses::CUI_Curses()
	: m_impl(std::make_unique<Impl>())
	{
	}

	CUI_Curses::~CUI_Curses()
	{
	}

	KString CUI_Curses::getName() const
	{
		return "Curses";
	}

	void CUI_Curses::init()
	{
		m_impl->init();
	}

	void CUI_Curses::refreshConnectionList()
	{
		m_impl->refreshConnectionList();
	}
}
