/*
 * ecv_rrf.h
 *
 *  Created on: 31 Oct 2021
 *      Author: David
 *
 *  This is the file we include instead of including ecv.h directly.
 *  It includes the standard ecv.h and then #undefs those macros which cause problems for C++ standard include files.
 */

#ifndef SRC_GENERAL_ECV_RRF_H_
#define SRC_GENERAL_ECV_RRF_H_

#include "ecv_original.h"

#undef array	// used in C++ standard headers
#undef out
#undef value
#undef result	// used in cmsis_gcc.h
#undef from		// used in RP2040 SDK

// C++ doesn't define a 16-bit floating point type, so eCv uses _ecv_float16_t

#ifdef __ECV__
typedef _ecv_float16_t __fp16;		// define ARM GCC's floating type to be the same as the eCv builtin one
#endif

#endif /* SRC_GENERAL_ECV_RRF_H_ */
