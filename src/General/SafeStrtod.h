/*
 * SafeStrtod.h
 *
 *  Created on: 8 Apr 2018
 *      Author: David
 */

#ifndef SRC_LIBRARIES_GENERAL_SAFESTRTOD_H_
#define SRC_LIBRARIES_GENERAL_SAFESTRTOD_H_

#include "../ecv_duet3d.h"
#include <cstdint>

float SafeStrtof(const char *_ecv_array s, const char *_ecv_array *null endptr = nullptr) noexcept;

uint32_t StrToU32(const char *_ecv_array s, const char *_ecv_array *null endptr = nullptr) noexcept;

// This overload is used by the 12864 menu code in RepRapFirmware
inline uint32_t StrToU32(char *_ecv_array s, char *_ecv_array *null endptr = nullptr) noexcept
{
	// Defining it this way saves duplicating the code
	return StrToU32(s, const_cast<const char *_ecv_array *null>(endptr));
}

int32_t StrToI32(const char *_ecv_array s, const char *_ecv_array *null endptr = nullptr) noexcept;
uint32_t StrOptHexToU32(const char *_ecv_array s, const char *_ecv_array *null endptr = nullptr) noexcept;
uint32_t StrHexToU32(const char *_ecv_array s, const char *_ecv_array *null endptr = nullptr) noexcept;

#define strtod(s, p) Do_not_use_strtod_use_SafeStrtof_instead(s, p)
#define strtof(s, p) Do_not_use_strtof_use_SafeStrtof_instead(s, p)
#define strtol(s, ...) Do_not_use_strtol_use_StrToI32_instead(s, __VA_ARGS__)
#define strtoul(s, ...) Do_not_use_strtoul_use_StrToU32_instead(s, __VA_ARGS__)
#define atof(s) Do_not_use_atof_use_SafeStrtof_instead(s)
#define atoi(s) Do_not_use_atoi_use_StrToI32_instead(s)
#define atol(s) Do_not_use_atol_use_StrToU32_instead(s)

#endif /* SRC_LIBRARIES_GENERAL_SAFESTRTOD_H_ */
