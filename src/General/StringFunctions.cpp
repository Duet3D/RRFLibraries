/*
 * StringFunctions.cpp
 *
 *  Created on: 8 Jul 2019
 *      Author: David
 */

#include "StringFunctions.h"
#include <cstring>
#include <cctype>

bool StringEndsWithIgnoreCase(c_string string, c_string ending) noexcept
{
	(void)strlen(string);
	const size_t j = strlen(string);
	const size_t k = strlen(ending);
	return k <= j && StringEqualsIgnoreCase(string + (j - k), ending);
}

bool StringEqualsIgnoreCase(c_string s1, c_string s2) noexcept
{
	size_t i = 0;
	while (s1[i] != 0 && s2[i] != 0)
	keep(	i < s1.lim;
			i < s2.lim;
			forall k _ecv_in 0..((_ecv_integer)i-1) :- s1[k] != 0 && s2[k] != 0;
			i <= strlen(s1);
			i <= strlen(s2);
			forall k _ecv_in 0..((_ecv_integer)i-1) :- tolower((int)s1[k]) == tolower((int)s2[k]))
	decrease((_ecv_integer)strlen(s1) - i)
	{
		if (tolower((int)s1[i]) != tolower((int)s2[i]))
		{
			return false;
		}
		i++;
	}

	return s1[i] == 0 && s2[i] == 0;
}

bool ReducedStringEquals(c_string s1, c_string s2) noexcept
{
	while (*s1 != 0 && *s2 != 0)
	keep(s1.base == (old s1).base; s1 < old s1 + (old s1).lim; s1 <= (old s1) + strlen(old s1);
		s2.base == (old s2).base; s2 < old s2 + (old s2).lim; s2 <= (old s2) + strlen(old s2);
		_ecv_isNullTerminated(s1))
	decrease(strlen(s1) + strlen(s2))
	{
		if (*s1 == '-' || *s1 == '_')
		{
			++s1;
		}
		else if (*s2 == '-' || *s2 == '_')
		{
			++s2;
		}
		else if (tolower((int)*s1) != tolower((int)*s2))
		{
			return false;
		}
		else
		{
			++s1;
			++s2;
		}
	}

	return *s1 == 0 && *s2 == 0;
}

bool StringStartsWith(c_string string, c_string starting) noexcept
{
	while (*starting != 0)
	keep(starting.base == (old starting).base; starting < old starting + (old starting).lim; starting <= (old starting) + strlen(old starting);
		 string.base == (old string).base; string < old string + (old string).lim; string <= (old string) + strlen(old string);
		 starting - old starting == string - old string;
		 forall i _ecv_in 0..((starting - old starting) - 1) :- (old starting)[i] == (old string)[i]
		)
	decrease(strlen(string))
	{
		if (*starting != *string)
		{
			return false;
		}
		++starting;
		++string;
	}

	return true;
}

bool StringStartsWithIgnoreCase(c_string string, c_string starting) noexcept
{
	while (*starting != 0)
	keep(starting.base == old(starting).base; string.base == old(string).base; nullTerm(starting); nullTerm(string))
	decrease(strlen(string))
	{
		if (tolower((int)*starting) != tolower((int)*string))
		{
			return false;
		}
		++starting;
		++string;
	}

	return true;
}

int StringContains(c_string string, c_string match) noexcept
{
	int i = 0;
	int count = 0;

	while (string[i] != 0)
	keep(i <= strlen(string); forall j _ecv_in 0..(i - 1) :- j + strlen(match) > strlen(string) || ntData(string).drop(j).take(strlen(match)) != ntData(match))
	decrease(strlen(string) - i)
	{
		if (string[i++] == match[count])
		{
			count++;
			if (match[count] == 0)
			{
				return i - count;
			}
		}
		else
		{
			count = 0;
		}
	}

	return -1;
}

// Version of strncpy that ensures the result is null terminated
void SafeStrncpy(char *_ecv_array dst, c_string src, size_t length) noexcept
{
	strncpy(dst, src, length);
	dst[length - 1] = 0;
}

// Version of strcat that takes the original buffer size as the limit and ensures the result is null terminated
void SafeStrncat(char *_ecv_array dst, c_string src, size_t length) noexcept
{
	dst[length - 1] = 0;
	const size_t index = strlen(dst);
	strncat(dst + index, src, length - index);
	dst[length - 1] = 0;
}

// End
