/**
 * @file
 * @brief DialogConnectionColumns class.
 */

#pragma once

#include <array>
#include <vector>

#include "KString.hpp"
#include "Screen.hpp"

namespace ctp
{
	class ScreenConnectionList;

	class ConnectionColumn
	{
	public:
		enum EType
		{
			PROTO,
			PROTO_STATE,

			SRC_ADDRESS,
			SRC_PORT,
			SRC_COUNTRY,
			SRC_ASN,
			SRC_ASN_ORG_NAME,

			DST_ADDRESS,
			DST_PORT,
			DST_COUNTRY,
			DST_ASN,
			DST_ASN_ORG_NAME,

			RX_PACKETS,
			RX_BYTES,
			RX_SPEED,
			TX_PACKETS,
			TX_BYTES,
			TX_SPEED
		};

		static constexpr unsigned int COLUMN_COUNT = 18;

		static unsigned int GetDefaultWidth(EType column);
		static KString GetLabel(EType column);
		static KString GetName(EType column);

	private:
		EType m_type;
		unsigned int m_width;
		bool m_hasLabel;

	public:
		ConnectionColumn(EType type, unsigned int width, bool hasLabel = true)
		: m_type(type),
		  m_width(width),
		  m_hasLabel(hasLabel)
		{
		}

		EType getType() const
		{
			return m_type;
		}

		unsigned int getWidth() const
		{
			return m_width;
		}

		unsigned int getDefaultWidth() const
		{
			return GetDefaultWidth(m_type);
		}

		KString getLabel() const
		{
			return (m_hasLabel) ? GetLabel(m_type) : KString();
		}

		KString getName() const
		{
			return GetName(m_type);
		}
	};

	class DialogConnectionColumns : public Screen
	{
		class ColumnConfig
		{
			ConnectionColumn::EType m_type;
			unsigned int m_width;
			bool m_isEnabled;

		public:
			ColumnConfig(ConnectionColumn::EType type, bool isEnabled)
			: m_type(type),
			  m_width(ConnectionColumn::GetDefaultWidth(type)),
			  m_isEnabled(isEnabled)
			{
			}

			ConnectionColumn::EType getType() const
			{
				return m_type;
			}

			unsigned int getWidth() const
			{
				return m_width;
			}

			bool isEnabled() const
			{
				return m_isEnabled;
			}

			KString getName() const
			{
				return ConnectionColumn::GetName(m_type);
			}

			void setType(ConnectionColumn::EType type)
			{
				m_type = type;
			}

			void setWidth(unsigned int width)
			{
				m_width = width;
			}

			void setEnabled(bool isEnabled)
			{
				m_isEnabled = isEnabled;
			}
		};

		std::vector<ConnectionColumn> m_columns;
		std::array<ColumnConfig, ConnectionColumn::COLUMN_COUNT> m_columnConfig;
		std::array<ColumnConfig, ConnectionColumn::COLUMN_COUNT> m_columnConfigOld;
		unsigned int m_cursorPos;

		void applyConfig();
		void restoreConfig();
		void draw();
		void drawEntry(unsigned int index);
		void fillEmpty(int count);
		void fillEmpty();

		void handleResize() override;
		bool handleKey(int ch) override;

	public:
		DialogConnectionColumns(ScreenConnectionList *parent);

		void open();
		void close(bool apply = false);

		const std::vector<ConnectionColumn> & getColumns() const
		{
			return m_columns;
		}

		unsigned int getColumnsTotalWidth() const
		{
			unsigned int width = 0;

			for (const ConnectionColumn & column : m_columns)
			{
				width += column.getWidth();
				width += 1;  // column separator
			}

			if (width > 0)
			{
				width -= 1;  // remove last column separator
			}

			return width;
		}

		bool isColumnEnabled(ConnectionColumn::EType type) const
		{
			for (const ConnectionColumn & column : m_columns)
			{
				if (column.getType() == type)
				{
					return true;
				}
			}

			return false;
		}
	};
}
