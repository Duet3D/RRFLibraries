/*
 * StringRef.h
 *
 *  Created on: 10 Jan 2016
 *      Author: David
 */

#ifndef STRINGREF_H_
#define STRINGREF_H_

#include <cstddef>	// for size_t
#include <cstdarg>	// for va_args
#include <cstring>	// for strlen

#include "Strnlen.h"
#include "StringFunctions.h"

// Class to describe a string buffer, including its length. This saves passing buffer lengths around everywhere.
class StringRef
{
	char * _ecv_array p;	// pointer to the storage
	size_t len;				// number of characters in the storage, must be at least 1

	//invariant(len >= 1; p.lim >= len; exists i in 0..len-1 :- p[0] == 0)

public:
	StringRef(char * _ecv_array pp, size_t pl) noexcept writes() : p(pp), len(pl) { }

	size_t Capacity() const noexcept { return len - 1; }
	size_t strlen() const noexcept;
	bool IsEmpty() const noexcept { return p[0] == 0; }

	c_string c_str() const noexcept { return p; }
	char * _ecv_array Pointer() const noexcept { return p; }						// use Pointer() only in the very care case that we need direct write access to the storage!

	char& operator[](size_t index) const noexcept { return p[index]; }

	void Clear() const noexcept writes(p.all) { p[0] = 0; }

	int printf(c_string fmt, ...) const noexcept writes(p.all) __attribute__ ((format (printf, 2, 3)));
	int vprintf(c_string fmt, va_list vargs) const noexcept writes(p.all);
	int catf(c_string fmt, ...) const noexcept  writes(p.all) __attribute__ ((format (printf, 2, 3)));
	int lcatf(c_string fmt, ...) const noexcept  writes(p.all) __attribute__ ((format (printf, 2, 3)));
	int vcatf(c_string fmt, va_list vargs) const noexcept writes(p.all);
	bool copy(c_string src) const noexcept writes(p.all);										// returns true if buffer is too small
	bool copy(const char *_ecv_array src, size_t maxlen) const noexcept writes(p.all);		// returns true if buffer is too small
	bool cat(c_string src) const noexcept writes(p.all);										// returns true if buffer is too small
	bool lcat(c_string src) const noexcept writes(p.all);										// returns true if buffer is too small
	bool catn(const char *_ecv_array src, size_t n) const noexcept writes(p.all);				// returns true if buffer is too small
	bool lcatn(const char *_ecv_array src, size_t n) const noexcept writes(p.all);			// returns true if buffer is too small
	bool cat(char c) const noexcept writes(p.all);											// returns true if buffer is too small
	size_t StripTrailingSpaces() const noexcept writes(p.all);
	bool Prepend(c_string src) const noexcept writes(p.all);									// returns true if buffer is too small
	void Truncate(size_t pos) const noexcept writes(p.all);
	void Erase(size_t pos, size_t count = 1) const noexcept writes(p.all);
	bool Insert(size_t pos, char c) const noexcept writes(p.all);								// returns true if buffer is too small
	bool Insert(size_t pos, c_string s) const noexcept writes(p.all);							// returns true if buffer is too small
	bool Equals(c_string s) const noexcept { return strcmp(p, s) == 0; }
	bool EqualsIgnoreCase(c_string s) const noexcept { return StringEqualsIgnoreCase(p, s); }
	int Contains(c_string s) const noexcept;
	int Contains(char c) const noexcept;
	bool Replace(char oldVal, char newVal) const noexcept writes(p.all);						// replace the first instance of oldVal by newVal
	unsigned int ReplaceAll(char oldVal, char newVal) const noexcept writes(p.all);			// replace all instances of oldVal by newVal
};

#endif /* STRINGREF_H_ */
