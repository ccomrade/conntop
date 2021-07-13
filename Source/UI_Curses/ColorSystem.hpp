/**
 * @file
 * @brief ColorSystem class.
 */

#pragma once

class ColorSystem
{
public:
	enum EAttributeType
	{
		ATTR_DEFAULT = 0,
		ATTR_IMPORTANT_RED,
		ATTR_IMPORTANT_GREEN,
		ATTR_FUNCTION_KEY_BAR_LABEL,
		ATTR_FUNCTION_KEY_BAR_NAME,
		ATTR_STATUS_LABEL,
		ATTR_STATUS_VALUE,
		ATTR_LIST_HEADER,
		ATTR_SELECTED_ROW,
		ATTR_UNRESOLVED,
		ATTR_UNRESOLVED_SELECTED,
		ATTR_TRAFFIC,
		ATTR_TRAFFIC_SELECTED,
		ATTR_PROTO_UDP,
		ATTR_PROTO_UDP_SELECTED,
		ATTR_PROTO_TCP,
		ATTR_PROTO_TCP_SELECTED,
		ATTR_COUNTRY_CODE,
		ATTR_COUNTRY_CODE_SELECTED
	};

private:
	enum EColorPair
	{
		DEFAULT = 0,  // automatically initialized
		RED_ON_DEFAULT,
		RED_ON_CYAN,
		GREEN_ON_DEFAULT,
		GREEN_ON_CYAN,
		YELLOW_ON_DEFAULT,
		YELLOW_ON_CYAN,
		CYAN_ON_DEFAULT,
		BLACK_ON_CYAN,
		BLACK_ON_GREEN,
		DEFAULT_ON_CYAN,

		_COLOR_PAIR_MAX  // this should not be higher than 256, see manual page of COLOR_PAIR
	};

	static const int ATTRIBUTE_TABLE[];
	static const unsigned int ATTRIBUTE_TABLE_SIZE;

public:
	ColorSystem();

	int getAttr(EAttributeType attr) const
	{
		return ATTRIBUTE_TABLE[attr];
	}

	static void Init();
};

extern ColorSystem *gColorSystem;
