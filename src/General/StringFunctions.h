/*
 * StringFunctions.h
 *
 *  Created on: 8 Jul 2019
 *      Author: David
 */

#ifndef SRC_GENERAL_STRINGFUNCTIONS_H_
#define SRC_GENERAL_STRINGFUNCTIONS_H_

#include "../ecv_duet3d.h"
#include <cstdint>
#include <cstddef>

bool StringEndsWithIgnoreCase(c_string string, c_string ending) noexcept
pre (nullTerm(string); nullTerm(ending));

bool StringStartsWith(c_string string, c_string starting) noexcept
pre(nullTerm(string); nullTerm(starting));

bool StringStartsWithIgnoreCase(c_string string, c_string starting) noexcept
pre(nullTerm(string); nullTerm(starting));

bool StringEqualsIgnoreCase(c_string s1, c_string s2) noexcept
pre(nullTerm(s1); nullTerm(s2));

bool ReducedStringEquals(c_string s1, c_string s2) noexcept
pre(nullTerm(s1); nullTerm(s2));

int StringContains(c_string str, c_string match) noexcept
pre(nullTerm(string); nullTerm(match));

void SafeStrncpy(char *_ecv_array dst, c_string src, size_t length) noexcept
pre(length != 0; dst.lim >= length; nullTerm(src); disjoint(src.all, dst.all))
post(src.all == old src.all; (strlen(src) < length) ? (forall i in 0..(strlen(src) + 1) :- dst[i] == src[i]) : (forall i in 0..(length - 2) :- dst[i] == src[i]) && dst[length - 1] == 0;
	 nullTerm(dst)
	);

void SafeStrncat(char *_ecv_array dst, c_string src, size_t length) noexcept
pre(length != 0; dst.lim >= length; nullTerm(src); nullTerm(dst))
post(src.all == old src.all;
	 forall i in 0..(strlen(old dst) - 1) :- dst[i] == old dst[i];
	 (strlen(old dst) + strlen(src) < length) ? (forall i in 0..strlen(src) :- dst[strlen(old dst) + i] == src[i]) : (forall i in 0..(length - strlen(old dst) - 1) :- dst[strlen(old dst) + i] == src[i]) && dst[length - 1] == 0;
	 nullTerm(dst)
	);

#endif /* SRC_GENERAL_STRINGFUNCTIONS_H_ */
