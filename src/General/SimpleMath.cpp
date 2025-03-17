/*
 * SimpleMath.cpp
 *
 *  Created on: 16 Mar 2025
 *      Author: David
 */

#include "SimpleMath.h"
#include <cmath>

extern "C" void debugPrintf(const char *fmt, ...) noexcept;

float fastCubeRootf(float f) noexcept
{
	if (f == 0.0) { return 0.0; }						// estimating the reciprocal cube root fails if the operand is zero

	// First calculate the approximate value of f^-(2/3).
	// See "Generalising the Fast Reciprocal Square Root Algorithm" by Mike Day, https://arxiv.org/pdf/2307.15600
	const float f2 = fsquare(f);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	const uint32_t i = *reinterpret_cast<const uint32_t*>(&f2);
	const uint32_t i2 = 0x54B8E38E - i/3;
	const float y = *reinterpret_cast<const float*>(&i2);
#pragma GCC diagnostic pop
	const float z = f2 * y * y * y;
	const float f1 = y * (1.3739948 - z * (0.47285829 - z * 0.092823250));

	// f1 is now an approximation of f^-(2/3). Multiply by f to get an approximation of f^(1/3).
	const float r1 = f1 * f;

	// Do 2 Newton-Raphson iterations to improve the accuracy of the result.
	// To avoid division, instead of dividing by (3 * r^2) we multiply by (f1 * 1/3) because f1 is an approximation of f^-(2/3) and therefore an approximation of 1/r^2.
	const float r2 = r1 - (fcube(r1) - f) * f1 * (1.0/3.0);
	const float ret = r2 - (fcube(r2) - f) * f1 * (1.0/3.0);

#if 0		// debug to check the result
	const float aret = fabsf(ret);
	const float nextRet = std::nextafter(aret, 2 * aret);
	const float nextCube = fcube(nextRet);
	const float prevRet = std::nextafter(aret, 0.0);
	const float prevCube = fcube(prevRet);
	if (   ret * f < 0.0								// if the sign of the result is incorrect
		|| nextCube < fabsf(f)
		|| prevCube > fabsf(f)
	   )
	{
		const float r3 = ret - (fcube(ret) - f) * f1 * (1.0/3.0);
		debugPrintf("%7e %7e %7e %7e %7e %7e %7e %7e %7e\n", (double)ret, (double)f, (double)prevRet, (double)prevCube, (double)nextRet, (double)nextCube, (double)r1, (double)r2, (double)r3);
	}
#endif

	return ret;
}

// End
