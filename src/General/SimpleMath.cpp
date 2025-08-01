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

#if (defined(SAME70) && SAME70) || defined(__SAME70Q21__)

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

// Note, we can easily underflow the range of a float (about +/- 3.4e-38) in this function, therefore we use double arithmetic.
size_t SolveCubic(float fa, float fb, float fc, float fd, float *rslt) noexcept
{
	const double a = (double)fa;
	const double b = (double)fb;
	const double c = (double)fc;
	const double d = (double)fd;
	if (a == (double)0.0)
	{
		// The equation is actually quadratic
		const double discriminant = dsquare((double)c) - 4 * b * d;
		if (discriminant == (double)0.0)
		{
			rslt[0] = -c/(2 * b);
			return 1;
		}
		if (discriminant > (double)0.0)
		{
			const double s = (float)fastSqrtd(discriminant);
			rslt[0] = (float)((s - c)/(2 * b));
			rslt[1] = (float)(-(s + c)/(2 * b));
			return 2;
		}
		return 0;
	}
	else
	{
		const double bc = b * c;
		const double ad = a * d;
		const double a2d = ad * a;
		const double abc = a * bc;
		const double threeA = 3 * a;
		const double delta0 = dsquare(b) - (threeA * c);
		const double delta1 = (2 * dcube(b)) - (abc * 9) + (27 * a2d);
		if (delta0 == (double)0.0)
		{
			if (delta1 == (double)0.0)
			{
				// One real root with multiplicity 3
				rslt[0] = (float)(-b/threeA);
				return 1;
			}

			// Else the discriminant must be positive and we have one real root
			const double bigC = fastCubeRootf(delta1);
			rslt[0] = (float)(-(b + bigC)/threeA);
			return 1;
		}

		const double minusDiscriminant = dsquare(delta1) - 4 * dcube(delta0);
//		debugPrintf("md=%.3e\n", minusDiscriminant);
		if (minusDiscriminant == (double)0.0)
		{
			debugPrintf("d0=%.4e d1=%.4e\n", delta0, delta1);
			// We have one real root with multiplicity 2 and one other real root
			rslt[0] = (float)(((9 * ad) - bc)/(2 * delta0));								// root with multiplicity 2
			rslt[1] = (float)(((4 * abc) - (9 * a2d) - dcube(b))/(a * delta0));				// simple root
			return 2;
		}

		if (minusDiscriminant > (double)0.0)
		{
			// One real root and two complex conjugate roots
			const double bigC = fastCubeRootd((delta1 + fastSqrtd(minusDiscriminant)) * (double)0.5);
			rslt[0] = (float)(-(b + bigC + delta0/bigC)/threeA);
			return 1;
		}

		// Else there are three real roots and we need complex arithmetic (or equivalently, trigonometry) to find them
		const std::complex<double> cube((double)0.5 * delta1, (double)0.5 * fastSqrtd(-minusDiscriminant));
		// Instead of evaluating fastCubeRootf(abs(cube)) in the following we could take the 6th root of norm(cube), which should be a little faster but needs more code
		const std::complex<double> bigC0 = std::polar<float>(fastCubeRootd(abs(cube)), arg(cube)/(double)3.0);
        const std::complex<double> cbrtMinus1 = std::complex<float>(-(double)0.5, (double)0.5 * sqrt((double)3.0));
		const std::complex<double> bigC1 = bigC0 * cbrtMinus1;
		const std::complex<double> bigC2 = bigC0 * conj(cbrtMinus1);
		rslt[0] = (float)(-(b + bigC0.real() + (delta0/bigC0).real())/threeA);
		rslt[1] = (float)(-(b + bigC1.real() + (delta0/bigC1).real())/threeA);
		rslt[2] = (float)(-(b + bigC2.real() + (delta0/bigC2).real())/threeA);
		return 3;
	}
}

// Return the smallest non-negative root of the equation
float SmallestNonNegativeCubicSolution(float a, float b, float c, float d) noexcept
{
	float rslt[3];
	const size_t numSolutions = SolveCubic(a, b, c, d, rslt);
	debugPrintf("%u solutions:", numSolutions);	//***TEMP!
	if (numSolutions >= 1) { debugPrintf(" %.3e", (double)rslt[0]); }
	if (numSolutions >= 2) { debugPrintf(" %.3e", (double)rslt[1]); }
	if (numSolutions >= 3) { debugPrintf(" %.3e", (double) rslt[2]); }
	debugPrintf("\n");
	switch (numSolutions)
	{
	case 3:
		if (rslt[2] >= 0.0 && (rslt[1] < 0.0 || rslt[2] < rslt[1]) && (rslt[0] < 0.0 || rslt[2] < rslt[0])) { return rslt[2]; }
		//[[fallthrough]]
		// no break
	case 2:
		if (rslt[1] >= 0.0 && (rslt[0] < 0.0 || rslt[1] < rslt[0])) { return rslt[1]; }
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

#endif

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
