/*
 * StringRef.cpp
 *
 *  Created on: 10 Jan 2016
 *      Author: David
 */

#include "StringRef.h"
#include <cstring>
#include <cstdio>
#include "SafeVsnprintf.h"

//*************************************************************************************************
// StringRef class member implementations

size_t StringRef::strlen() const noexcept
{
	return Strnlen(p, len - 1);
}

int StringRef::printf(const char *_ecv_array fmt, ...) const noexcept
{
	va_list vargs;
	va_start(vargs, fmt);
	const int ret = SafeVsnprintf(p, len, fmt, vargs);
	va_end(vargs);
	return ret;
}

int StringRef::vprintf(const char *_ecv_array fmt, va_list vargs) const noexcept
{
	return SafeVsnprintf(p, len, fmt, vargs);
}

int StringRef::catf(const char *_ecv_array fmt, ...) const noexcept
{
	const size_t n = strlen();
	if (n + 1 < len)		// if room for at least 1 more character and a null
	{
		va_list vargs;
		va_start(vargs, fmt);
		const int ret = SafeVsnprintf(p + n, len - n, fmt, vargs);
		va_end(vargs);
		return (ret < 0) ? ret : ret + (int)n;
	}
	return 0;
}

// This is like catf but it adds a newline first if the string being appended to is not empty. Useful for building error messages that may describe more than one error.
int StringRef::lcatf(const char *_ecv_array fmt, ...) const noexcept
{
	size_t n = strlen();
	if (n != 0 && p[n - 1] != '\n')
	{
		if (cat('\n'))
		{
			return 0;
		}
		++n;
	}
	if (n + 1 < len)		// if room for at least 1 more character and a null
	{
		va_list vargs;
		va_start(vargs, fmt);
		const int ret = SafeVsnprintf(p + n, len - n, fmt, vargs);
		va_end(vargs);
		return (ret < 0) ? ret : ret + (int)n;
	}
	return 0;
}

int StringRef::vcatf(const char *_ecv_array fmt, va_list vargs) const noexcept
{
	const size_t n = strlen();
	if (n + 1 < len)		// if room for at least 1 more character and a null
	{
		const int ret = SafeVsnprintf(p + n, len - n, fmt, vargs);
		return (ret < 0) ? ret : ret + (int)n;
	}
	return 0;
}

// This is quicker than printf for printing constant strings
bool StringRef::copy(const char *_ecv_array src) const noexcept
{
	const size_t slen = ::strlen(src);
	const bool overflow = (slen >= len);
	const size_t length = (overflow) ? len - 1 : slen;
	memcpy(p, src, length);
	p[length] = 0;
	return overflow;
}

// This is quicker than printf for copying constant strings
bool StringRef::copy(const char *_ecv_array src, size_t maxlen) const noexcept
{
	const size_t slen = Strnlen(src, maxlen);
	const bool overflow = (slen >= len);
	const size_t length = (overflow) ? len - 1 : slen;
	memcpy(p, src, length);
	p[length] = 0;
	return overflow;
}

// This is quicker than catf for appending constant strings
bool StringRef::cat(const char *_ecv_array src) const noexcept
{
	const size_t length = strlen();
	const size_t slen = ::strlen(src);
	const bool overflow = (length + slen >= len);
	const size_t toCopy = (overflow) ? len - length - 1 : slen;
	memcpy(p + length, src, toCopy);
	p[length + toCopy] = 0;
	return overflow;
}

// As cat but add a newline first if the string being appended to is not empty and doesn't end in newline
bool StringRef::lcat(const char *_ecv_array src) const noexcept
{
	if (!IsEmpty() && p[strlen() - 1] != '\n')
	{
		if (cat('\n'))
		{
			return true;
		}
	}
	return cat(src);
}

// Concatenate with a limit on the number of characters read
bool StringRef::catn(const char *_ecv_array src, size_t n) const noexcept
{
	const size_t length = strlen();
	const size_t slen = Strnlen(src, n);
	const bool overflow = (length + slen >= len);
	const size_t toCopy = (overflow) ? len - length - 1 : slen;
	memcpy(p + length, src, toCopy);
	p[length + toCopy] = 0;
	return overflow;
}

// As catn but add a newline first if the string being appended to is not empty and doesn't end in newline
bool StringRef::lcatn(const char *_ecv_array src, size_t n) const noexcept
{
	if (!IsEmpty() && p[strlen() - 1] != '\n')
	{
		if (cat('\n'))
		{
			return true;
		}
	}
	return catn(src, n);
}

// Append a character
bool StringRef::cat(char c) const noexcept
{
	const size_t length = strlen();
	if (length + 1 < len)
	{
		p[length] = c;
		p[length + 1] = 0;
		return false;
	}
	return true;
}

// Remove trailing spaces from the string and return its new length
size_t StringRef::StripTrailingSpaces() const noexcept
{
	size_t slen = strlen();
	while (slen != 0 && p[slen - 1] == ' ')
	{
		--slen;
		p[slen] = 0;
	}
	return slen;
}

bool StringRef::Prepend(const char *_ecv_array src) const noexcept
{
	const size_t slen = ::strlen(src);
	const size_t dlen = strlen();
	if (slen + dlen < len)
	{
		memmove(p + slen, p, dlen + 1);
		memcpy(p, src, slen);
		return false;
	}
	return true;
}

void StringRef::Truncate(size_t pos) const noexcept
{
	if (pos < len)
	{
		p[pos] = 0;
	}
}

void StringRef::Erase(size_t pos, size_t count) const noexcept
{
	const size_t slen = strlen();
	if (pos < slen)
	{
		while (pos + count < slen)
		{
			p[pos] = p[pos + count];
			++pos;
		}
		p[pos] = 0;
	}
}

// Insert a character, returning true if the string was truncated
bool StringRef::Insert(size_t pos, char c) const noexcept
{
	const size_t slen = strlen();
	if (pos > slen)
	{
		return false;										// insert point is out of range, but return success anyway
	}

	if (slen + 1 < len)										// check there is space for the existing string + null + inserted character
	{
		// There is space for the extra character
		memmove(p + pos + 1, p + pos, slen - pos + 1);		// copy the data up including the null terminator
		p[pos] = c;
		return false;
	}

	if (pos < slen)
	{
		// The buffer is already full, but we haven't been asked to insert the character right at the end
		memmove(p + pos + 1, p + pos, slen - pos - 1);		// leave the null terminator intact and drop the last character
		p[pos] = c;
	}
	return true;
}

// Insert another string, returning true if the string was truncated
bool StringRef::Insert(size_t pos, const char *_ecv_array s) const noexcept
{
	const size_t slen = strlen();
	if (pos > slen)
	{
		return false;										// insert point is out of range, but return success anyway
	}

	const size_t slen2 = ::strlen(s);
	if (slen + slen2 < len)									// check there is space for the existing string + null + inserted characters
	{
		// There is space for the extra characters
		memmove(p + pos + slen2, p + pos, slen - pos + 1);	// copy the data up including the null terminator
		memcpy(p + pos, s, slen2);
		return false;
	}

	if (pos < slen)
	{
		// The buffer doesn't have enough room, but we haven't been asked to insert the characters right at the end
		if (pos + slen2  < len)
		{
			// There is room for the whole inserted string and maybe for some of the characters of the original string after the inserted string
			memmove(p + pos + slen2, p + pos, len - pos - slen2);		// leave the null terminator intact and drop the last characters
			memcpy(p + pos, s, slen2);
		}
		else
		{
			// We can only copy part of the inserted string
			memcpy(p + pos, s, len - pos - 1);
		}
		p[len - 1] = 0;
	}
	return true;
}

int StringRef::Contains(const char *_ecv_array s) const noexcept
{
	const char *_ecv_array null const r = strstr(p, s);
	return (r == nullptr) ? -1 : not_null(r) - p;
}

int StringRef::Contains(char c) const noexcept
{
	const char *_ecv_array null const r = strchr(p, (int)c);
	return (r == nullptr) ? -1 : not_null(r) - p;
}

// Replace the first instance of oldVal by newVal, returning true if a replacement was done
bool StringRef::Replace(char oldVal, char newVal) const noexcept
{
	for (size_t i = 0; i < len && p[i] != 0; ++i)
	{
		if (p[i] == oldVal)
		{
			p[i] = newVal;
			return true;
		}
	}
	return false;
}

// Replace all instances of oldVal by newVal, returning the number of replacements done
unsigned int StringRef::ReplaceAll(char oldVal, char newVal) const noexcept
{
	unsigned int replacements = 0;
	for (size_t i = 0; i < len && p[i] != 0; ++i)
	{
		if (p[i] == oldVal)
		{
			p[i] = newVal;
			++replacements;
		}
	}
	return replacements;
}

// End
