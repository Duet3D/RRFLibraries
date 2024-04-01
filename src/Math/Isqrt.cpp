// Fast 62-bit integer square root algorithms

#include "Isqrt.h"

#ifdef __SAMC21G18A__

# include <Core.h>
# include <component/divas.h>

// Fast 62-bit integer square root function (thanks dmould)
__attribute__((aligned(8)))			// align on a flash cache line boundary
uint32_t isqrt64(uint64_t num) noexcept
{
	uint32_t numHigh = (uint32_t)(num >> 32);
	uint32_t numLow = (uint32_t)num;
	if (numHigh == 0)
	{
		// 32-bit square root. Use the DIVAS to calculate it.
		// We need to disable interrupts to prevent other tasks or ISRs using the DIVAS at the same time.
		const irqflags_t flags = IrqSave();
		DIVAS->SQRNUM.reg = numLow;
		while (DIVAS->STATUS.bit.BUSY) { }
		const uint32_t rslt = DIVAS->RESULT.reg;
		IrqRestore(flags);
		// eCv doesn't know the semantics of the DIVAS unit to we need to tell it
		_ecv_assume(rslt * rslt <= numLow; (rslt + 1) * (rslt + 1) > numLow);
		return rslt;
	}

	if ((numHigh & (3u << 30)) != 0)
	{
		// Input out of range - probably negative, so return -1
		return 0xFFFFFFFF;
	}

	// 62-bit square root. Use the DIVAS to calculate the top 30 bits of the result and the remainder.
	uint32_t res, rem;
	{
		const irqflags_t flags = IrqSave();
		DIVAS->SQRNUM.reg = numHigh;
		while (DIVAS->STATUS.bit.BUSY) { }
		res = DIVAS->RESULT.reg;
		rem = DIVAS->REM.reg;
		IrqRestore(flags);
	}
	// eCv doesn't know the semantics of the DIVAS unit to we need to tell it
	_ecv_assume(res * res <= numHigh; (res + 1) * (res + 1) > numHigh; res * res + rem == numHigh);

	// At this point, res is the square root of the msw of the original number, in the range 0..2^16-2 with the input restricted to 62 bits
	// rem may have up to 16 bits set
	// On the SAMC21 I found 2 iterations per loop in the following to be faster than 3 or 4, probably because the SAMC21 flash cache is 64 bytes long.
	// This is the most optimum SAM21 code I managed to create. I didn't manage to persuade gcc to get rid of the compare instruction at the end of the loop.
	for (unsigned int i = 0; i < 8; ++i)
	{
		rem = (rem << 2) | (numLow >> 30);
		numLow <<= 2;
		res <<= 2;
		if (rem >= (res | 1u))
		{
			rem -= (res | 1u);
			res |= 2u;
		}

		rem = (rem << 2) | (numLow >> 30);
		numLow <<= 2;
		if (rem >= ((res << 1) | 1u))
		{
			rem -= ((res << 1) | 1u);
			res |= 1u;
		}
	}
	return res;
}

#else

// Fast 62-bit integer square root function (thanks dmould)
uint32_t isqrt64(uint64_t num) noexcept
{
	uint32_t numHigh = (uint32_t)(num >> 32);
	if (numHigh == 0)
	{
		// 32-bit square root - thanks to Wilco Dijkstra for this efficient ARM algorithm
		uint32_t num32 = (uint32_t)num;
		uint32_t res = 0;

# define iter32(N)									\
		{											\
			const uint32_t temp = res | (1u << N);	\
			_ecv_assert(res < (1u << (15 - N)));	\
			if (num32 >= temp << N)					\
			{										\
				num32 -= temp << N;					\
				res |= 2u << N;						\
			}										\
			_ecv_assert(res < (1u << (N + 2)));		\
		}

		// We need to do 16 iterations
		iter32(15); iter32(14); iter32(13); iter32(12);
		iter32(11); iter32(10); iter32(9); iter32(8);
		iter32(7); iter32(6); iter32(5); iter32(4);
		iter32(3); iter32(2); iter32(1); iter32(0);

		return res >> 1;

# undef iter32

	}
	else if ((numHigh & (3u << 30)) != 0)
	{
		// Input out of range - probably negative, so return -1
		return 0xFFFFFFFFu;
	}
	else
	{
		// 62-bit square root
		uint32_t res = 0;

# define iter64a(N) 									\
		{												\
			_ecv_assert(res < (1u << (28 - N)));		\
			res <<= 1;									\
			const uint32_t temp = (res | 1u) << (N);	\
			_ecv_assert(temp < (1u << (N + 1)));		\
			if (numHigh >= temp)						\
			{											\
				numHigh -= temp;						\
				res |= 2u;								\
			}											\
		}

		// We need to do 15 iterations (not 16 because we have eliminated the top 2 bits)
					iter64a(28) iter64a(26) iter64a(24)
		iter64a(22) iter64a(20) iter64a(18) iter64a(16)
		iter64a(14) iter64a(12) iter64a(10) iter64a(8)
		iter64a(6)  iter64a(4)  iter64a(2)  iter64a(0)

		uint64_t numAll = ((uint64_t)numHigh << 32) | (uint32_t)num;
# undef iter64a

		// At this point, res is twice the square root of the msw, in the range 0..2^16-2 with the input restricted to 62 bits
		// numAll may have up to 24 bits set
		// On the SAME5x the following code is faster than the SAMC21 version (2.61us vs. 3.37us)
# define iter64b(N) 											\
		{														\
			res <<= 1;											\
			const uint64_t temp = (uint64_t)(res | 1u) << (N);	\
			if (numAll >= temp)									\
			{													\
				numAll -= temp;									\
				res |= 2u;										\
			}													\
		}

		// We need to do 16 iterations.
		// After the last iteration, numAll may be between 0 and (1 + 2 * res) inclusive.
		// So to take square roots of numbers up to 62 bits, we need to do all these iterations using 64 bit maths.
		// If we restricted the input to e.g. 48 bits, then we could do some of the final iterations using 32-bit maths.
		iter64b(30) iter64b(28) iter64b(26) iter64b(24)
		iter64b(22) iter64b(20) iter64b(18) iter64b(16)
		iter64b(14) iter64b(12) iter64b(10) iter64b(8)
		iter64b(6)  iter64b(4)  iter64b(2)  iter64b(0)
# undef iter64b

		return res >> 1;
	}
}

#endif

// The following is no longer used for the SAMC21 because Qfplib-M0 is faster (2.29us vs. 3.60us for this one).
// So now it's only used by SAM4S builds.
#if !((defined(__FPU_USED) && __FPU_USED != 0) || (defined (__VFP_FP__) && !defined(__SOFTFP__))) && !defined(__SAMC21G18A__)

// This is a fast floating point square root function for processors that don't have a FPU.
// It doesn't handle negative, infinite, NaN or denormalised operands correctly. For normal operands it usually returns exactly the same result as calling sqrtf().
// On the SAM4S it takes 1.69us compared to 3.25us for sqrtf. On the SAMC21 it takes 3.68us compared to 8.28us for sqrtf.
// CAUTION! This deliberately returns zero if the operand is negative. This is too allow for rounding errors in the step time calculation functions.
#ifdef __SAMC21G18A__
__attribute__((section(".time_critical")))
#endif
float fastSqrtf(float f) noexcept
{
	// 1. Represent the IEEE float as unsigned integer so that we can work on the bits.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	const int32_t operand = *reinterpret_cast<const int32_t*>(&f);
#pragma GCC diagnostic pop

	// 2. If negative, zero or negative zero, return zero.
	if (operand <= 0)
	{
		return 0.0;
	}

	// 3. Extract the 8-bit exponent and test for NaN or infinity
	const uint32_t uexponent = (uint32_t)operand >> 23;
	if (uexponent == 0xFF || uexponent == 0)
	{
		return f;					// it's a NaN or infinity or denormalised value, so return the original value
	}

	// 4. Extract the 23-bit fraction and add the implicit leading 1
	uint32_t fraction = ((uint32_t)operand & ((1u << 23) - 1)) | (1u << 23);

	// 4a. Subtract the +127 exponent bias
	int32_t exponent = (int32_t)uexponent - 127;
	_ecv_assert(exponent <= 127; exponent >= -127);

	// 5. Make the exponent even, also shift it left to get more result bits. This puts fraction in the range 2^30 to 2^32 - 2^8 + 1.
	fraction <<= 7 + ((uint32_t)exponent & 1u);

	// 6. Halve the exponent, which also gets rid of the odd bit if it is present. Shift right is arithmetic in gcc, so the sign is preserved.
	exponent >>= 1;

	// 7. Take the square root of the 32-bit fraction
# ifdef __SAMC21G18A__
	// Use the DIVAS to calculate it. This gets us 16 result digits in a few more than 32 clocks.
	// We need to disable interrupts to prevent other tasks or ISRs using the DIVAS at the same time.
	const irqflags_t flags = IrqSave();
	DIVAS->SQRNUM.reg = fraction;
	while (DIVAS->STATUS.bit.BUSY) { }
	uint32_t res = DIVAS->RESULT.reg;
	fraction = DIVAS->REM.reg;
	IrqRestore(flags);

	// We have 16 result digits but we want 24
	for (unsigned int i = 0; i < 4; ++i)
	{
		fraction <<= 2;
		res <<= 2;
		if (fraction >= (res | 1u))
		{
			fraction -= (res | 1u);
			res |= 2u;
		}

		fraction <<= 2;
		if (fraction >= ((res << 1) | 1u))
		{
			fraction -= ((res << 1) | 1u);
			res |= 1u;
		}
	}
# else
	// Thanks to Wilco Dijkstra for this efficient ARM algorithm
	uint32_t res = 0;

#  define iter32(N)								\
	{											\
		const uint32_t temp = res | (1u << N);	\
		if (fraction >= temp << N)				\
		{										\
			fraction -= temp << N;				\
			res |= 2u << N;						\
		}										\
	}

	iter32(15) iter32(14) iter32(13) iter32(12) iter32(11) iter32(10) iter32(9) iter32(8)
	iter32(7)  iter32(6)  iter32(5)  iter32(4) 	iter32(3)  iter32(2)  iter32(1)  iter32(0)

	// We have 16 binary digits but we want 24 including the leading 1.
	// The remainder may have 17 digits, so we can't shift it left by 16 and then do 8 iterations. So shift it left 14 and do 7 iterations.
	fraction <<= 14;
	res <<= 7;
	iter32(6) iter32(5) iter32(4) iter32(3) iter32(2) iter32(1) iter32(0)

#  undef iter32

	// Do one more iteration. We can avoid some shifting by doing it slightly differently.
	fraction <<= 2;
	{
		const uint32_t temp = (res << 1) | 1u;
		if (fraction >= temp)
		{
			fraction -= temp;
			res |= 1u;
		}
	}
# endif

	// Round the result. The remainder is in the range 0..(2 * res - 1)
	if (fraction > res)
	{
		++res;
	}

	// Assemble the result
	res &= (1u << 23) - 1;
	exponent += 127;
	res |= (uint32_t)exponent << 23;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	return *reinterpret_cast<const float *>(&res);
#pragma GCC diagnostic pop
}

#endif

// End
