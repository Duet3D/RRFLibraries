/*
 * NamedEnum.cpp
 *
 *  Created on: 7 Mar 2020
 *      Author: David
 */

#include "NamedEnum.h"
#include <cstddef>
#include <cstring>

// Function to search the table of names for a match. Returns numNames if not found.
unsigned int NamedEnumLookup(const char *_ecv_array s, const char *_ecv_array const names[], size_t numNames) noexcept
{
	unsigned int i = 0;
	while (i < numNames && strcmp(s, SkipLeadingUnderscore(names[i])) != 0)
	{
		++i;
	}
	return i;
}

// End
