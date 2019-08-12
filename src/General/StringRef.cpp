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

size_t StringRef::strlen() const
{
	return Strnlen(p, len - 1);
}

int StringRef::printf(const char *fmt, ...) const
{
	va_list vargs;
	va_start(vargs, fmt);
	const int ret = SafeVsnprintf(p, len, fmt, vargs);
	va_end(vargs);
	return ret;
}

int StringRef::vprintf(const char *fmt, va_list vargs) const
{
	return SafeVsnprintf(p, len, fmt, vargs);
}

int StringRef::catf(const char *fmt, ...) const
{
	const size_t n = strlen();
	if (n + 1 < len)		// if room for at least 1 more character and a null
	{
		va_list vargs;
		va_start(vargs, fmt);
		const int ret = SafeVsnprintf(p + n, len - n, fmt, vargs);
		va_end(vargs);
		return ret + n;
	}
	return 0;
}

int StringRef::vcatf(const char *fmt, va_list vargs) const
{
	const size_t n = strlen();
	if (n + 1 < len)		// if room for at least 1 more character and a null
	{
		return SafeVsnprintf(p + n, len - n, fmt, vargs) + n;
	}
	return 0;
}

// This is quicker than printf for printing constant strings
bool StringRef::copy(const char* src) const
{
	const size_t slen = ::strlen(src);
	const bool overflow = (slen >= len);
	const size_t length = (overflow) ? len - 1 : slen;
	memcpy(p, src, length);
	p[length] = 0;
	return overflow;
}

// This is quicker than printf for printing constant strings
bool StringRef::copy(const char* src, size_t maxlen) const
{
	const size_t slen = Strnlen(src, maxlen);
	const bool overflow = (slen >= len);
	const size_t length = (overflow) ? len - 1 : slen;
	memcpy(p, src, length);
	p[length] = 0;
	return overflow;
}

// This is quicker than catf for printing constant strings
bool StringRef::cat(const char* src) const
{
	const size_t length = strlen();
	const size_t slen = ::strlen(src);
	const bool overflow = (length + slen >= len);
	const size_t toCopy = (overflow) ? len - length - 1 : slen;
	memcpy(p + length, src, toCopy);
	p[length + toCopy] = 0;
	return overflow;
}

// Concatenate with a limit on the number of characters read
bool StringRef::catn(const char *src, size_t n) const
{
	const size_t length = strlen();
	const size_t slen = Strnlen(src, n);
	const bool overflow = (length + slen >= len);
	const size_t toCopy = (overflow) ? len - length - 1 : slen;
	memcpy(p + length, src, toCopy);
	p[length + toCopy] = 0;
	return overflow;
}

// Append a character
bool StringRef::cat(char c) const
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
size_t StringRef::StripTrailingSpaces() const
{
	size_t slen = strlen();
	while (slen != 0 && p[slen - 1] == ' ')
	{
		--slen;
		p[slen] = 0;
	}
	return slen;
}

bool StringRef::Prepend(const char *src) const
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

void StringRef::Truncate(size_t pos) const
{
	if (pos < len)
	{
		p[pos] = 0;
	}
}

void StringRef::Erase(size_t pos, size_t count) const
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
bool StringRef::Insert(size_t pos, char c) const
{
	const size_t slen = strlen();
	if (pos > slen)
	{
		return false;										// insert point is out of range, but return success anyway
	}
	else if (slen + 1 < len)
	{
		// There is space for the extra character
		memmove(p + pos + 1, p + pos, slen - pos + 1);		// copy the data up including the null terminator
		p[pos] = c;
		return false;
	}
	else if (pos < len)
	{
		// The buffer is full, but we haven't been asked to insert the character right at the end
		memmove(p + pos + 1, p + pos, slen - pos - 1);		// leave the null terminator intact
		p[pos] = c;
	}
	return true;
}

// End
