/*
 * Strnlen.h
 *
 *  Created on: 17 Apr 2018
 *      Author: David
 */

#ifndef SRC_LIBRARIES_GENERAL_STRNLEN_H_
#define SRC_LIBRARIES_GENERAL_STRNLEN_H_

#include "../ecv_duet3d.h"
#include <cstddef>
#include <cstring>

// 'strnlen' isn't ISO standard, so we define our own
size_t Strnlen(const char *_ecv_array s, size_t n) noexcept
pre(_ecv_isNullTerminated(s) || s.lim >= n)
post(r : r <= n; r <= s.lim; forall i _ecv_in 0..(r - 1) :- s[i] != 0; r == n || s[r] == 0);

#endif /* SRC_LIBRARIES_GENERAL_STRNLEN_H_ */
