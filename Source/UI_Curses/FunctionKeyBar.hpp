/**
 * @file
 * @brief FunctionKeyBar class.
 */

#pragma once

#include <string>
#include <array>

#include "KString.hpp"

namespace ctp
{
	class Screen;

	class FunctionKeyBar
	{
	public:
		enum EKey
		{
			F1, F2, F3, F4, F5, F6, F7, F8, F9, F10
		};

	private:
		struct FunctionKey
		{
			std::string name;
			bool isEnabled;
			bool isCustom;

			FunctionKey()
			: name(),
			  isEnabled(false),
			  isCustom(false)
			{
			}
		};

		Screen *m_parent;
		std::array<FunctionKey, 10> m_functionKeys;

		void initKey(EKey key, const KString & name)
		{
			FunctionKey & data = m_functionKeys[key];
			data.name = name;
			data.isEnabled = true;
			data.isCustom = false;
		}

	public:
		FunctionKeyBar(Screen *parent);

		void restoreKey(EKey key);
		bool handlePressedKey(int ch);
		void draw();

		bool isKeyEnabled(EKey key) const
		{
			return m_functionKeys[key].isEnabled;
		}

		bool isKeyCustom(EKey key) const
		{
			return m_functionKeys[key].isCustom;
		}

		KString getKeyName(EKey key) const
		{
			return (isKeyEnabled(key)) ? KString(m_functionKeys[key].name) : KString();
		}

		void setKeyEnabled(EKey key, bool isEnabled = true)
		{
			m_functionKeys[key].isEnabled = isEnabled;
		}

		void setCustomKey(EKey key, const KString & name)
		{
			FunctionKey & data = m_functionKeys[key];
			data.name = name;
			data.isEnabled = true;
			data.isCustom = true;
		}
	};
}
