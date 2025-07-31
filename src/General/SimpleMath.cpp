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

#define SACRIFICE_RANGE		(0)

float fastCubeRootf(float f) noexcept
{
	if (f == 0.0 || std::isnan(f) || std::isinf(f)) { return f; }						// estimating the reciprocal cube root fails if the operand is zero

	// First calculate the approximate value of f^-(2/3).
	// See "Generalising the Fast Reciprocal Square Root Algorithm" by Mike Day, https://arxiv.org/pdf/2307.15600
	// We need to either square the operand here (which gives the best accuracy), or square the intermediate result a few lines down (which gives greater range but lower initial accuracy)
#if SACRIFICE_RANGE
	const float f2 = fsquare(f);
#else
	const float f2 = fabsf(f);
#endif
	// If we switch to C++20 or later then we can use bit_cast instead of reinterpret_cast in the following, then we won't need these pragmas
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
	const uint32_t i = *reinterpret_cast<const uint32_t*>(&f2);
	const uint32_t i2 = 0x54B8E38E - i/3;
	const float y = *reinterpret_cast<const float*>(&i2);
#pragma GCC diagnostic pop
	const float z = f2 * y * y * y;
#if SACRIFICE_RANGE
	const float f1 = y * (1.3739948 - z * (0.47285829 - z * 0.092823250));
#else
	const float f1 = fsquare(y * (1.3739948 - z * (0.47285829 - z * 0.092823250)));
#endif

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
		debugPrintf("%7e %7e %7e %7e %7e %7e %7e %7e %7e %7e\n", (double)f1, (double)ret, (double)f, (double)prevRet, (double)prevCube, (double)nextRet, (double)nextCube, (double)r1, (double)r2, (double)r3);
	}
#endif

	return ret;
}

// Solve a cubic equation
// We are only interested in real solutions. The solutions are stored returned in rslt and the return value is the number of solutions.
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
		const float bc = b * c;
		const float ad = a * d;
		const float a2d = ad * a;
		const float abc = a * bc;
		const float threeA = 3 * a;
		const float delta0 = fsquare(b) - (threeA * c);
		const float delta1 = (2 * fcube(b)) - (abc * 9) + (27 * a2d);
		if (delta0 == 0.0)
		{
			if (delta1 == 0.0)
			{
				// One real root with multiplicity 3
				rslt[0] = -b/threeA;
				return 1;
			}

			// Else the discriminant must be positive and we have one real root
			const float bigC = fastCubeRootf(delta1);
			rslt[0] = -(b + bigC)/threeA;
			return 1;
		}

		const float minusDiscriminant = fsquare(delta1) - 4 * fcube(delta0);
		if (minusDiscriminant == 0.0)
		{
			// We have one real root with multiplicity 2 and one other real root
			rslt[0] = ((9 * ad) - bc)/(2 * delta0);									// root with multiplicity 2
			rslt[1] = ((4 * abc) - (9 * a2d) - fcube(b))/(a * delta0);				// simple root
			return 2;
		}

		if (minusDiscriminant > 0.0)
		{
			// One real root and two complex conjugate roots
			const float bigC = fastCubeRootf((delta1 + fastSqrtf(minusDiscriminant)) * 0.5);
			rslt[0] = -(b + bigC + delta0/bigC)/threeA;
			return 1;
		}

		// Else there are three real roots and we need complex arithmetic (or equivalently, trigonometry) to find them
		const std::complex<float> cube(0.5 * delta1, 0.5 * fastSqrtf(-minusDiscriminant));
		// Instead of evaluating fastCubeRootf(abs(cube)) in the following we could take the 6th root of norm(cube), which should be a little faster but needs more code
		const std::complex<float> bigC0 = std::polar<float>(fastCubeRootf(abs(cube)), arg(cube)/3.0);
        const std::complex<float> cbrtMinus1 = std::complex<float>(-0.5, 0.5 * sqrtf(3.0));
		const std::complex<float> bigC1 = bigC0 * cbrtMinus1;
		const std::complex<float> bigC2 = bigC0 * conj(cbrtMinus1);
		rslt[0] = -(b + bigC0.real() + (delta0/bigC0).real())/threeA;
		rslt[1] = -(b + bigC1.real() + (delta0/bigC1).real())/threeA;
		rslt[2] = -(b + bigC2.real() + (delta0/bigC2).real())/threeA;
		return 3;
	}
}

// Return the smallest non-negative root of the equation
float SmallestNonNegativeCubicSolution(float a, float b, float c, float d) noexcept
{
	float rslt[3];
	const size_t numSolutions = SolveCubic(a, b, c, d, rslt);
	debugPrintf("%u solutions\n", numSolutions);	//***TEMP!
	switch (numSolutions)
	{
	case 3:
		if (rslt[2] >= 0.0 && rslt[2] < rslt[1] && rslt[2] < rslt[0]) { return rslt[2]; }
		//[[fallthrough]]
		// no break
	case 2:
		if (rslt[1] >= 0.0 && rslt[1] < rslt[0]) { return rslt[1]; }
		//[[fallthrough]]
		// no break
	case 1:
		if (rslt[0] >= 0.0) { return rslt[0]; }
		//[[fallthrough]]
		// no break
	default:
		return std::numeric_limits<float>::quiet_NaN();
	}
}

// Return the smallest non-negative root of the equation. Returns the greatest root if both roots are negative.
float SmallestNonNegativeQuadraticSolution(float a, float b, float c) noexcept
{
	if (a == 0.0)
	{
		return -c/b;
	}
	const float temp = fastSqrtf(fsquare(b) - 4 * a * c);
	if (a < 0)
	{
		a = -a; b = -b;
	}
	return ((b + temp <= 0) ? -(b + temp) : (temp - b))/(2 * a);
}

// End
