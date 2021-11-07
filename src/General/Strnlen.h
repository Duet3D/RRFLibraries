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

// 'strnlen' isn't ISO standard, so we define our own
size_t Strnlen(const char *_ecv_array s, size_t n) noexcept;

#endif /* SRC_LIBRARIES_GENERAL_STRNLEN_H_ */
