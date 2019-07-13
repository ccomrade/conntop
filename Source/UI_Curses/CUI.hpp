/**
 * @file
 * @brief Curses UI.
 */

#pragma once

#include <memory>

#include "IUI.hpp"

namespace ctp
{
	class CUI_Curses : public IUI
	{
		class Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		CUI_Curses();
		~CUI_Curses();

		KString getName() const override;

		void init() override;

		void refreshConnectionList() override;
	};
}
