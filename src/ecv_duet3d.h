/*
 * ecv_rrf.h
 *
 *  Created on: 31 Oct 2021
 *      Author: David
 *
 *  This is the file we include instead of including ecv.h directly.
 */

#ifndef SRC_GENERAL_ECV_RRF_H_
#define SRC_GENERAL_ECV_RRF_H_

#include "ecv_original.h"

// Undefine eCv macros that are used in some C++ headers. Use the versions prefixed with "_ecv_" for these instead.
#undef array	// used in C++ standard headers
#undef out
#undef value
#undef result	// used in cmsis_gcc.h
#undef from		// used in RP2040 SDK

// C++ doesn't define a 16-bit floating point type, so eCv uses _ecv_float16_t

#ifdef __ECV__

// RRF typedefs float16_t to be ARM's __fp16. So define __fp16 to be the same type as the eCv builtin one.
typedef _ecv_float16_t __fp16;

#endif

// Define a type name to mean a C string that is guaranteed to be null terminated
typedef const char *_ecv_array _ecv_invariant(exists i::0.._ecv_value.upb :- _ecv_value[i] == 0) c_string;

#endif /* SRC_GENERAL_ECV_RRF_H_ */
