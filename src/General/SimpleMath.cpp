/*
 * SimpleMath.cpp
 *
 *  Created on: 16 Mar 2025
 *      Author: David
 */

#include "SimpleMath.h"
#include <cmath>
#include <complex>

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

// Solve a cubic equation
// We are only interested in real solutions. The solutions are stored returned in rslt and the return value is the number of solutions
// See https://en.wikipedia.org/wiki/Cubic_equation.
size_t SolveCubic(float a, float b, float c, float d, float *rslt) noexcept
{
	if (a == 0.0)
	{
		// The equation is actually quadratic
		const float discriminant = fsquare(c) - 4 * b * d;
		if (discriminant == 0.0)
		{
			rslt[0] = -c/(2 * b);
			return 1;
		}
		if (discriminant > 0.0)
		{
			const float s = fastSqrtf(discriminant);
			rslt[0] = (s - c)/(2 * b);
			rslt[1] = -(s + c)/(2 * b);
			return 2;
		}
		return 0;
	}
	else
	{
		const float delta0 = fsquare(b) - (a * c * 3);
		const float delta1 = (2 * fcube(b)) - (a * c * b * 9) + (27 * fsquare(a) * d);
		if (delta0 == 0.0)
		{
			if (delta1 == 0.0)
			{
				// One real root with multiplicity 3
				rslt[0] = -b/(3 * a);
				return 1;
			}

			// Else the discriminant must be positive and we have one real root
			const float bigC = fastCubeRootf(delta1);
			rslt[0] = -(b + bigC)/(3 * a);
			return 1;
		}

		const float minusDiscriminant = fsquare(delta1) - 4 * fcube(delta0);
		if (minusDiscriminant == 0.0)
		{
			// We have one real root with multiplicity 2 and one other real root
			rslt[0] = ((9 * a * d) - (b * c))/(2 * delta0);									// root with multiplicity 2
			rslt[1] = ((4 * a * b * c) - (9 * fsquare(a) * d) - fcube(b))/(a * delta0);		// simple root
			return 2;
		}

		if (minusDiscriminant > 0.0)
		{
			// One real root and two complex conjugate roots
			const float bigC = fastCubeRootf((delta1 + fastSqrtf(minusDiscriminant)) * 0.5);
			rslt[0] = -(b + bigC + delta0/bigC)/(3 * a);
			return 1;
		}

		// Else there are three real roots and we need complex arithmetic (or equivalently, trigonometry) to find them
		std::complex<float> cube(0.5 * delta1, 0.5 * fastSqrtf(-minusDiscriminant));
		std::complex<float> bigC0 = std::polar<float>(fastCubeRootf(abs(cube)), arg(cube)/3.0);
		std::complex<float> bigC1 = bigC0 * std::complex<float>(-0.5, 0.5 * sqrtf(3.0));
		std::complex<float> bigC2 = bigC0 * std::complex<float>(-0.5, -0.5 * sqrtf(3.0));
		rslt[0] = -(b + bigC0.real() + (delta0/bigC0).real())/(3 * a);
		rslt[1] = -(b + bigC1.real() + (delta0/bigC1).real())/(3 * a);
		rslt[2] = -(b + bigC2.real() + (delta0/bigC2).real())/(3 * a);
		return 3;
	}
}

// End
