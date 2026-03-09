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
#include <cctype>

bool StringEndsWithIgnoreCase(c_string string, c_string ending) noexcept
pre(nullTerm(string); nullTerm(ending));

bool StringStartsWith(c_string string, c_string starting) noexcept
pre(nullTerm(string); nullTerm(starting))
_ecv_returns(ntData(string).begins(ntData(starting)));

bool StringStartsWithIgnoreCase(c_string string, c_string starting) noexcept
pre(nullTerm(string); nullTerm(starting));

bool StringEqualsIgnoreCase(c_string s1, c_string s2) noexcept
pre(nullTerm(s1); nullTerm(s2))
_ecv_returns(strlen(s1) == strlen(s2) && (forall i _ecv_in 0..((_ecv_integer)strlen(s1) - 1) :- tolower((int)s1[i]) == tolower((int)s2[i])));

bool ReducedStringEquals(c_string s1, c_string s2) noexcept
pre(nullTerm(s1); nullTerm(s2));

int StringContains(c_string string, c_string match) noexcept
pre(nullTerm(string); nullTerm(match))
post(r :   (r == -1 && !(exists i _ecv_in 0..((_ecv_integer)strlen(string) - (_ecv_integer)strlen(match)) :- ntData(string).drop(i).take(strlen(match)) == ntData(match)))
     || (r _ecv_in 0..((_ecv_integer)strlen(string) - (_ecv_integer)strlen(match)) && ntData(string).drop(r).take(strlen(match)) == ntData(match))
	);

void SafeStrncpy(char *_ecv_array dst, c_string src, size_t length) noexcept
pre(length != 0; dst.lim >= length; nullTerm(src); disjoint(src.all, dst.all))
post(src.all == old src.all;
	 nullTerm(dst);
     ntData(dst) == ((strlen(src) < length)
						? ntData(src)
						: ntData(src).take(length - 1)
					)
	);

void SafeStrncat(char *_ecv_array dst, c_string src, size_t length) noexcept
pre(length != 0; dst.lim >= length; nullTerm(src); nullTerm(dst); strlen(dst) < length; disjoint(src.all, dst.all))
post(src.all == old src.all;
	 nullTerm(dst);
	 ntData(dst) == ((old(strlen(dst)) + strlen(src) < length)
					 ? (old(ntData(dst))).concat(ntData(src))
					 : (old(ntData(dst))).concat(ntData(src)).take(length - 1)
					)
	);

#endif /* SRC_GENERAL_STRINGFUNCTIONS_H_ */
