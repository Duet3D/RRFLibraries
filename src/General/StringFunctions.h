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

bool StringEndsWithIgnoreCase(const char* _ecv_array string, const char* _ecv_array ending) noexcept;
bool StringStartsWith(const char* _ecv_array string, const char* _ecv_array starting) noexcept;
bool StringStartsWithIgnoreCase(const char* _ecv_array string, const char* _ecv_array starting) noexcept;
bool StringEqualsIgnoreCase(const char* _ecv_array s1, const char* _ecv_array s2) noexcept;
bool ReducedStringEquals(const char* _ecv_array s1, const char* _ecv_array s2) noexcept;
int StringContains(const char* _ecv_array string, const char* _ecv_array match) noexcept;
void SafeStrncpy(char * _ecv_array dst, const char * _ecv_array src, size_t length) noexcept pre(length != 0);
void SafeStrncat(char * _ecv_array dst, const char * _ecv_array src, size_t length) noexcept pre(length != 0);

#endif /* SRC_GENERAL_STRINGFUNCTIONS_H_ */
