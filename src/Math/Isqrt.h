/*
 * Isqrt.h
 *
 *  Created on: 12 Aug 2016
 *      Author: David
 */

#ifndef SRC_LIBRARIES_MATH_ISQRT_H_
#define SRC_LIBRARIES_MATH_ISQRT_H_

#include <cstdint>

extern uint32_t isqrt64(uint64_t num) noexcept;		// This is defined in its own file, Isqrt.cpp or Isqrt.asm

#if !((defined(__FPU_USED) && __FPU_USED) || (defined (__VFP_FP__) && !defined(__SOFTFP__)))
float fastSqrtf(float f) noexcept;
#endif

#endif /* SRC_LIBRARIES_MATH_ISQRT_H_ */
