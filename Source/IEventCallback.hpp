/**
 * @file
 * @brief IEventCallback class.
 */

#pragma once

template<class T>
struct IEventCallback
{
	virtual void onEvent(const T & event) = 0;
};
