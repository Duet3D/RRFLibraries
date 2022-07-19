/*
 * String.h
 *
 *  Created on: 8 Jan 2020
 *      Author: David
 */

#ifndef SRC_GENERAL_STRING_H_
#define SRC_GENERAL_STRING_H_

#include "StringRef.h"

// Class to describe a string which we can access directly or get a StringRef reference to
template<size_t Len> class String
{
public:
	String() noexcept { storage[0] = 0; }

	StringRef GetRef() noexcept { return StringRef(storage, Len + 1); }
	const char *_ecv_array c_str() const noexcept { return storage; }
	size_t strlen() const noexcept { return Strnlen(storage, Len); }
	bool IsEmpty() const noexcept { return storage[0] == 0; }
	bool IsFull() const noexcept { return strlen() == Len; }
	char& operator[](size_t index) noexcept { return storage[index]; }
	char operator[](size_t index) const noexcept { return storage[index]; }
	constexpr size_t Capacity() const noexcept { return Len; }
	bool EndsWith(char c) const noexcept;

	void Clear() noexcept { storage[0] = 0; }
	int printf(const char *_ecv_array fmt, ...) noexcept __attribute__ ((format (printf, 2, 3)));
	int vprintf(const char *_ecv_array fmt, va_list vargs) noexcept;
	int catf(const char *_ecv_array fmt, ...) noexcept __attribute__ ((format (printf, 2, 3)));
	int vcatf(const char *_ecv_array fmt, va_list vargs) noexcept;
	bool copy(const char *_ecv_array src) noexcept { return GetRef().copy(src); }	// returns true if buffer is too small
	bool copy(const char *_ecv_array src, size_t maxlen) noexcept { return GetRef().copy(src, maxlen); }	// returns true if buffer is too small
	bool cat(const char *_ecv_array src) noexcept { return GetRef().cat(src); }		// returns true if buffer is too small
	bool catn(const char *_ecv_array src, size_t n) noexcept { return GetRef().catn(src, n); }		// returns true if buffer is too small
	bool cat(char c) noexcept { return GetRef().cat(c); }				// returns true if buffer is too small
	bool Prepend(const char *_ecv_array src) noexcept;								// returns true if buffer is too small

	void CopyAndPad(const char *_ecv_array src) noexcept;
	bool ConstantTimeEquals(String<Len> other) const noexcept;

	bool Replace(char oldVal, char newVal) noexcept { return GetRef().Replace(oldVal, newVal); }
	unsigned int ReplaceAll(char oldVal, char newVal) noexcept { return GetRef().ReplaceAll(oldVal, newVal); }
	void Truncate(size_t len) noexcept;
	void Erase(size_t pos, size_t count = 1) noexcept;
	bool Insert(size_t pos, char c) noexcept { return GetRef().Insert(pos, c); }		// returns true if buffer is too small
	bool Insert(size_t pos, const char *_ecv_array s) noexcept { return GetRef().Insert(pos, s); }	// returns true if buffer is too small
	bool Equals(const char *_ecv_array s) const noexcept { return strcmp(storage, s) == 0; }
	bool EqualsIgnoreCase(const char *_ecv_array s) const noexcept { return StringEqualsIgnoreCase(storage, s); }
	// Compare with a C string. If the C string is too long but the part of it we could accommodate matches, return true.
	// this will ignore the \0 at the end
	bool Similar(const char *_ecv_array s) const noexcept { return strncmp(s, storage, Len) == 0; }
	int Contains(const char *_ecv_array s) const noexcept;
	int Contains(char c) const noexcept;

	char *_ecv_array Pointer() noexcept { return storage; }							// use this one only exceptionally and with great care!
	void EnsureNullTerminated() noexcept { storage[Len] = 0; }

private:
	char storage[Len + 1];
};

// Copy some text into this string and pad it with nulls so we can do a constant time compare
template<size_t Len> void String<Len>::CopyAndPad(const char *_ecv_array src) noexcept
{
	memset(storage, 0, Len + 1);
	copy(src);
}

// Do a constant time compare. Both this string and the other one much be padded with nulls.
template<size_t Len> bool String<Len>::ConstantTimeEquals(String<Len> other) const noexcept
{
	uint8_t rslt = 0;
	for (size_t i = 0; i < Len; ++i)
	{
		rslt |= ((uint8_t)storage[i] ^ (uint8_t)other.storage[i]);
	}
	return rslt == 0;
}

template<size_t Len> inline int String<Len>::vprintf(const char *_ecv_array fmt, va_list vargs) noexcept
{
	return GetRef().vprintf(fmt, vargs);
}

template<size_t Len> inline int String<Len>::vcatf(const char *_ecv_array fmt, va_list vargs) noexcept
{
	return GetRef().vcatf(fmt, vargs);
}

template<size_t Len> inline bool String<Len>::Prepend(const char *_ecv_array src) noexcept
{
	return GetRef().Prepend(src);
}

template<size_t Len> int String<Len>::printf(const char *_ecv_array fmt, ...) noexcept
{
	va_list vargs;
	va_start(vargs, fmt);
	const int ret = GetRef().vprintf(fmt, vargs);
	va_end(vargs);
	return ret;
}

template<size_t Len> int String<Len>::catf(const char *_ecv_array fmt, ...) noexcept
{
	va_list vargs;
	va_start(vargs, fmt);
	const int ret = GetRef().vcatf(fmt, vargs);
	va_end(vargs);
	return ret;
}

template<size_t Len> void String<Len>::Truncate(size_t len) noexcept
{
	if (len < Len)
	{
		storage[len] = 0;
	}
}

template<size_t Len> void String<Len>::Erase(size_t pos, size_t count) noexcept
{
	const size_t len = strlen();
	if (pos < len)
	{
		while (pos + count < len)
		{
			storage[pos] = storage[pos + count];
			++pos;
		}
		storage[pos] = 0;
	}
}

template<size_t Len> bool String<Len>::EndsWith(char c) const noexcept
{
	const size_t len = strlen();
	return len != 0 && storage[len - 1] == c;
}

template<size_t Len> int String<Len>::Contains(const char *_ecv_array s) const noexcept
{
	const char * _ecv_array null const p = strstr(storage, s);
	return (p == nullptr) ? -1 : not_null(p) - storage;
}

template<size_t Len> int String<Len>::Contains(char c) const noexcept
{
	const char * _ecv_array null const p = strchr(storage, (int)c);
	return (p == nullptr) ? -1 : not_null(p) - storage;
}

#endif /* SRC_GENERAL_STRING_H_ */
