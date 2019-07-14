/*
 * StringFunctions.h
 *
 *  Created on: 8 Jul 2019
 *      Author: David
 */

#ifndef SRC_GENERAL_STRINGFUNCTIONS_H_
#define SRC_GENERAL_STRINGFUNCTIONS_H_

#include "ecv.h"
#include <cstdint>
#include <cstddef>

bool StringEndsWithIgnoreCase(const char* string, const char* ending);
bool StringStartsWith(const char* string, const char* starting);
bool StringStartsWithIgnoreCase(const char* string, const char* starting);
bool StringEqualsIgnoreCase(const char* s1, const char* s2);
bool ReducedStringEquals(const char* s1, const char* s2);
int StringContains(const char* string, const char* match);
void SafeStrncpy(char *dst, const char *src, size_t length) pre(length != 0);
void SafeStrncat(char *dst, const char *src, size_t length) pre(length != 0);

#endif /* SRC_GENERAL_STRINGFUNCTIONS_H_ */
