/**
 * @file
 * @brief Implementation of WhoisData class.
 */

#include "WhoisData.hpp"

static const WhoisData EMPTY_UNKNOWN_WHOIS_DATA;

const WhoisData & WhoisData::GetEmptyUnknown()
{
	return EMPTY_UNKNOWN_WHOIS_DATA;
}
