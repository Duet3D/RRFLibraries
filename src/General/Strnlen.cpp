/*
 * Strnlen.cpp
 *
 *  Created on: 17 Apr 2018
 *      Author: David
 */

#include "Strnlen.h"

// Need to define strnlen here because it isn't ISO standard
size_t Strnlen(const char *_ecv_array s, size_t n) noexcept
{
	size_t rslt = 0;
	while (rslt < n && s[rslt] != 0)
	{
		++rslt;
	}
	return rslt;
}

// End
