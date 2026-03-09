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

	//invariant(_ecv_isNullTerminated(p))

public:
	// Declare an invariant we can use as a precondition and postcondition for the member functions.
	// Note that we require the maximum length to be <= maxof(int) so that functions such as printf can return either -1 if an error occurred or the number of characters stored.
	ghost(
		bool Invariant() const noexcept returns(len >= 1 && len <= maxof(int) && p.lim >= len && (exists i _ecv_in 0..(len - 1) :- p[i] == 0));
	)

	StringRef(char * _ecv_array pp, size_t pl) noexcept
		writes()
		pre(pl >= 1; pl <= maxof(int); pp.lim >= pl; exists i _ecv_in 0..(pl - 1) :- pp[i] == 0)
		post(Invariant())
		: p(pp), len(pl) { }

	size_t Capacity() const noexcept
		pre(Invariant())
		returns(len - 1)
		{ return len - 1; }

	size_t strlen() const noexcept
		pre(Invariant())
		post(r : r <= len - 1; forall i _ecv_in 0..(r - 1) :- p[i] != 0; r == len - 1 || p[r] == 0);

	bool IsEmpty() const noexcept
		pre(Invariant())
		returns(p[0] == 0)
		{ return p[0] == 0; }

	c_string c_str() const noexcept
		pre(Invariant())
		returns(p)
		post(r : _ecv_isNullTerminated(r))
		{ return p; }

	char *_ecv_array Pointer() const noexcept
		pre(Invariant())
		returns(p)
		{ return p; }														// use Pointer() only in the very rare case that we need direct write access to the storage!

	char& operator[](size_t index) const noexcept pre(Invariant(); index <= strlen()) { return p[index]; }

	void Clear() const noexcept
		writes(p.all) pre(Invariant())
		post(p[0] == 0; Invariant())
		{ p[0] = 0; }

	int printf(c_string fmt, ...) const noexcept __attribute__((format (printf, 2, 3)))
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(fmt)) post(Invariant());
	int vprintf(c_string fmt, va_list vargs) const noexcept
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(fmt)) post(Invariant());
	int catf(c_string fmt, ...) const noexcept __attribute__((format (printf, 2, 3)))
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(fmt); disjoint(p.all, fmt.all)) post(Invariant());
	int lcatf(c_string fmt, ...) const noexcept __attribute__((format (printf, 2, 3)))
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(fmt); disjoint(p.all, fmt.all)) post(Invariant());
	int vcatf(c_string fmt, va_list vargs) const noexcept
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(fmt); disjoint(p.all, fmt.all)) post(Invariant());
	bool copy(c_string src) const noexcept									// returns true if buffer is too small
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(src); disjoint(p.all, src.all)) post(Invariant());
	bool copy(const char *_ecv_array src, size_t maxlen) const noexcept		// returns true if buffer is too small
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(src); disjoint(p.all, src.all)) post(Invariant());
	bool cat(c_string src) const noexcept									// returns true if buffer is too small
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(src); disjoint(p.all, src.all)) post(Invariant());
	bool lcat(c_string src) const noexcept									// returns true if buffer is too small
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(src); disjoint(p.all, src.all)) post(Invariant());
	bool catn(const char *_ecv_array src, size_t n) const noexcept			// returns true if buffer is too small
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(src); disjoint(p.all, src.all)) post(Invariant());
	bool lcatn(const char *_ecv_array src, size_t n) const noexcept			// returns true if buffer is too small
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(src); disjoint(p.all, src.all)) post(Invariant());
	bool cat(char c) const noexcept											// returns true if buffer is too small
		writes(p.all) pre(Invariant()) post(Invariant());
	size_t StripTrailingSpaces() const noexcept
		writes(p.all) pre(Invariant()) post(Invariant());
	bool Prepend(c_string src) const noexcept								// returns true if buffer is too small
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(src); disjoint(p.all, src.all)) post(Invariant());
	void Truncate(size_t pos) const noexcept
		writes(p.all) pre(Invariant()) post(Invariant());
	void Erase(size_t pos, size_t count = 1) const noexcept
		writes(p.all) pre(Invariant()) post(Invariant());
	bool Insert(size_t pos, char c) const noexcept							// returns true if buffer is too small
		writes(p.all) pre(Invariant()) post(Invariant());
	bool Insert(size_t pos, c_string s) const noexcept						// returns true if buffer is too small
		writes(p.all) pre(Invariant(); _ecv_isNullTerminated(s); disjoint(p.all, s.all)) post(Invariant());
	bool Equals(c_string s) const noexcept
		pre(Invariant(); _ecv_isNullTerminated(s))
		{ return strcmp(p, s) == 0; }
	bool EqualsIgnoreCase(c_string s) const noexcept
		pre(Invariant(); _ecv_isNullTerminated(s))
		{ return StringEqualsIgnoreCase(p, s); }
	int Contains(c_string s) const noexcept
		pre(Invariant(); _ecv_isNullTerminated(s));
	int Contains(char c) const noexcept
		pre(Invariant());
	bool Replace(char oldVal, char newVal) const noexcept					// replace the first instance of oldVal by newVal
		writes(p.all) pre(Invariant()) post(Invariant());
	unsigned int ReplaceAll(char oldVal, char newVal) const noexcept		// replace all instances of oldVal by newVal
		writes(p.all) pre(Invariant()) post(Invariant());
};

#endif /* STRINGREF_H_ */
