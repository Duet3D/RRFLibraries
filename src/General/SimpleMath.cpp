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
	if (   std::signbit(ret( != std::signbit(f))								// if the sign of the result is incorrect
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

// Sort the result vector of an equation. This returns the number of elements passed so that functions that return the number of roots can chain to it.
static size_t SortRoots(double *rslt, size_t numRoots) noexcept
pre( numRoots <= 4)
{
	if (numRoots > 1)
	{
		if (rslt[0] > rslt[1])
		{
			std::swap(rslt[0], rslt[1]);
		}
		if (numRoots > 2)
		{
			if (numRoots == 4)
			{
				if (rslt[2] > rslt[3])
				{
					std::swap(rslt[2], rslt[3]);
				}
				if (rslt[1] > rslt[3])
				{
					std::swap(rslt[1], rslt[3]);
				}
			}
			if (rslt[0] > rslt[2])
			{
				std::swap(rslt[0], rslt[2]);
			}
			if (rslt[1] > rslt[2])
			{
				std::swap(rslt[1], rslt[2]);
			}
		}
	}
	return numRoots;
}

// Solve a quadratic equation using double arithmetic. We are only interested in real solutions. Returns the number of real solutions. The solutions are returned in rslt in increasing order.
size_t SolveQuadratic(double a, double b, double c, double rslt[2]) noexcept
{
	if (a == (double)0.0)
	{
		// The equation is linear
		rslt[0] = -(c/b);
		return 1;
	}
	const double discriminant = dsquare(b) - 4 * a * c;
	if (discriminant == (double)0.0)
	{
		rslt[0] = -b/(2 * a);
		return 1;
	}
	else if (discriminant > (double)0.0)
	{
		const double s = fastSqrtd(discriminant);
		rslt[0] = (s - b)/(2 * a);
		rslt[1] = -(s + b)/(2 * a);
		return SortRoots(rslt, 2);
	}
	else
	{
		return 0;
	}
}

// Solve a cubic equation
// We are only interested in real solutions. The solutions are stored returned in rslt and the return value is the number of solutions.
// See https://en.wikipedia.org/wiki/Cubic_equation.

// Note, we can easily underflow the range of a float (about +/- 3.4e-38) in this function.
// In particular, the b coefficient gets raised to the sixth power via intermediate values delta0 and delta1.
// As the b coefficient typically represents an acceleration in mm/step_clock^2 it can easily underflow, e.g. 1000 mm/sec^2 ~= 1e-3 mm/step_clock^2
// Therefore we use double arithmetic.

// The result vector is in ascending order. Returns the number of roots.
size_t SolveCubic(double a, double b, double c, double d, double rslt[3]) noexcept
{
	if (a == (double)0.0)
	{
		// The equation is actually quadratic
		return SolveQuadratic(b, c, d, rslt);
	}
	else if (d == (double)0.0)
	{
		// x=0 is a solution, which we can factor out
		if (c == (double)0.0)
		{
			// x = 0 is a double solution
			rslt[0] = (double)0.0;
			if (b == (double)0.0)
			{
				// x = 0 is a triple solution
				return 1;
			}
			else
			{
				rslt[1] = -b/a;
				return 2;
			}
		}
		const size_t numQuadraticSolutions = SolveQuadratic(a, b, c, rslt);
		rslt[numQuadraticSolutions] = (double)0.0;
		return SortRoots(rslt, numQuadraticSolutions + 1);
	}
	else
	{
		// It's tempting to divide the equation by 3*a here to avoid several divisions by 3*a.
		// However, if we do that then the case of the discriminant being zero in our test case in RepRapFirmware doesn't occur because of rounding error.
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
				rslt[0] = -b/threeA;
				return 1;
			}

			// Else the discriminant must be positive and we have one real root
			const double bigC = fastCubeRootd(delta1);
			rslt[0] = -(b + bigC)/threeA;
			return 1;
		}

		const double minusDiscriminant = dsquare(delta1) - 4 * dcube(delta0);
		if (minusDiscriminant == (double)0.0)
		{
			// We have one real root with multiplicity 2 and one other real root
			rslt[0] = ((9 * ad) - bc)/(2 * delta0);									// root with multiplicity 2
			rslt[1] = ((4 * abc) - (9 * a2d) - dcube(b))/(a * delta0);				// simple root
			return SortRoots(rslt, 2);
		}

		if (minusDiscriminant > (double)0.0)
		{
			// One real root and two complex conjugate roots. We just want the real one.
			const double mdsqrt = fastSqrtd(minusDiscriminant);
			// In the following, in principle we can use either the positive or the negative square root.
			// However if we choose the one that makes 'temp' in the following very small then we can get underflow,
			// which results in bigC becoming zero or nearly zero, and rslt[0] becomes infinite.
			// So we choose the sign of the square root to maximise abs(temp).
			const double temp = (std::signbit(delta1)) ? delta1 - mdsqrt : delta1 + mdsqrt;
			const double bigC = fastCubeRootd(temp * (double)0.5);
			rslt[0] = -(b + bigC + delta0/bigC)/threeA;
			return 1;
		}

		// Else there are three real roots and we need complex arithmetic (or equivalently, trigonometry) to find them
		const double phi = atan2(fastSqrtd(-minusDiscriminant), delta1)/(double)3.0;
		const double absC0 = fastSqrtd(delta0);
#if 1
		// We can make use of the fact that all of bigC0, bigC1 and bigC2 have magnitude absC0 = sqrt(delta0). This greatly simplifies the complex division operations.
		rslt[0] = -(b + 2 * absC0 * cos(phi))/threeA;
		rslt[1] = -(b - absC0 * (cos(phi) + sqrt((double)3.0) * sin(phi)))/threeA;
		rslt[2] = -(b - absC0 * (cos(phi) - sqrt((double)3.0) * sin(phi)))/threeA;
#else
		const std::complex<double> bigC0 = std::polar<double>(absC0, phi);
        const std::complex<double> cbrtMinus1 = std::complex<double>(-(double)0.5, (double)0.5 * sqrt((double)3.0));
		const std::complex<double> bigC1 = bigC0 * cbrtMinus1;
		const std::complex<double> bigC2 = bigC0 * conj(cbrtMinus1);
		rslt[0] = -(b + bigC0.real() + (delta0/bigC0).real())/threeA;
		rslt[1] = -(b + bigC1.real() + (delta0/bigC1).real())/threeA;
		rslt[2] = -(b + bigC2.real() + (delta0/bigC2).real())/threeA;
#endif
		return SortRoots(rslt, 3);
	}
}

#define DEBUG_QUARTIC	(0)

// Solve a quartic equation. We are only interested in real solutions. Returns the number of real solutions. The solutions are returned in rslt in increasing order.
// See https://en.wikipedia.org/wiki/Quartic_function#General_formula_for_roots
size_t SolveQuartic(double a, double b, double c, double d, double e, double rslt[4]) noexcept
{
	if (a == (double)0.0)
	{
		return SolveCubic(b, c, d, e, rslt);
	}

	// Simplify by making the coefficient of X^4 equal to 1
	b /= a;
	c /= a;
	d /= a;
	e /= a;

	// Convert to a depressed quartic: y^4 + py^2 + qy + r = 0 where y = x + b/4
	const double p = (8 * c - 3 * dsquare(b)) * ((double)1.0/(double)8.0);
	const double q = (dcube(b) - 4 * c * b + 8 * d) * ((double)1.0/(double)8.0);
	const double r = (- 3 * dsquare(dsquare(b)) + 256 * e - 64 * b * d + 16 * dsquare(b) * c) * ((double)1.0/(double)256.0);
#if DEBUG_QUARTIC
	debugPrintf("pqr = %.7g %.7g %.7g\n", p, q, r);
#endif

	if (q == 0)
	{
		// The equation is biquadratic: y^4 + py^2 + r = 0 i.e. quadratic in y^2
#if DEBUG_QUARTIC
		debugPrintf("bi-quadratic case\n");
#endif
		double tempResult[2];
		const size_t numQuadraticSolutions = SolveQuadratic(1.0, p, r, tempResult);
		if (numQuadraticSolutions == 0)
		{
			return 0;
		}
		size_t numSolutions = 0;
		if (tempResult[0] == (double)0.0)
		{
			rslt[numSolutions++] = -(double)0.25 * b;
		}
		else if (tempResult[0] > (double)0.0)
		{
			rslt[numSolutions++] = -fastSqrtd(tempResult[0]) - (double)0.25 * b;
			rslt[numSolutions++] = fastSqrtd(tempResult[0]) - (double)0.25 * b;
		}
		if (numQuadraticSolutions == 2)
		{
			if (tempResult[1] == (double)0.0)
			{
				rslt[numSolutions++] = -(double)0.25 * b;
			}
			else if (tempResult[1] > (double)0.0)
			{
				rslt[numSolutions++] = -fastSqrtd(tempResult[1]) - (double)0.25 * b;
				rslt[numSolutions++] = fastSqrtd(tempResult[1]) - (double)0.25 * b;
			}
		}
		return SortRoots(rslt, numSolutions);
	}
	else
	{
		// Not bi-quadratic
		const double delta0 = dsquare(c) - 3 * b * d + 12 * e;
		const double delta1 = 2 * dcube(c) - 9 * b * c * d + 27 * (dsquare(b) * e + dsquare(d)) - 72 * c * e;
		const double minusDiscriminant = dsquare(delta1) - 4 * dcube(delta0);
#if DEBUG_QUARTIC
		debugPrintf("md = %.7g\n", minusDiscriminant);
#endif
		if (minusDiscriminant < (double)0.0)
		{
			// Four real roots or no real roots
			const double phi = acos(delta1/2 * fastSqrtd(dcube(delta0)));
			const double TwoSsquared = (2 * fastSqrtd(delta0) * cos(phi/(double)3.0) - 2 * p)/(double)3.0;
			if (TwoSsquared < (double)0.0)
			{
				return 0;				// no solutions
			}
			const double S = (double)0.5 * fastSqrtd(TwoSsquared);
			const double minusBoverFour = -(double)0.25 * b;
			const double temp1 = (double)0.5 * fastSqrtd(-2 * (TwoSsquared + p) - q/S);
			const double temp2 = (double)0.5 * fastSqrtd(-2 * (TwoSsquared + p) + q/S);
			rslt[0] = minusBoverFour - S - temp1;
			rslt[1] = minusBoverFour - S + temp1;
			rslt[2] = minusBoverFour + S - temp2;
			rslt[3] = minusBoverFour + S + temp2;
			return SortRoots(rslt, 4);
		}
		else if (delta0 == (double)0.0 && delta1 == (double)0.0)
		{
#if DEBUG_QUARTIC
			debugPrintf("At least 3 equal roots\n");
#endif
			// We have at least three equal roots.
			// Therefore: (y-s)^3(y-t) is the same polynomial as y^4 + py^2 + qy + r
			// Therefore: y^4 - (3s + t)y^3 + 3s(s + t)y^2 - s^2(3t + s) + s^3t is the same polynomial as y^4 + py^2 + qy + r
			// From the coefficients of  y^3, t = -3s
			// From the coefficients of y^2, 3s(s + t) = p therefore 6s^2 = -p
			// From the coefficients of y, -(3t + s) = q therefore 8s^3 = q
			if (p > (double)0.0)
			{
				return 0;
			}

			const double minusBoverFour = -(double)0.25 * b;
			// We need to pick the correct square root, which we do by choosing the root with the same sign as p
			const double root = std::copysign(fastSqrtd(-p/(double)6.0), q);
			rslt[0] = minusBoverFour + std::copysign(root, q);
			rslt[1] = minusBoverFour - (double)3.0 * root;
			return SortRoots(rslt, 2);
		}
		else
		{
			const double TwoQcubed = (delta0 == (double)0.0) ? 2 * delta1
										: (delta1 < (double)0.0) ? delta1 - fastSqrtd(minusDiscriminant)
											: delta1 + fastSqrtd(minusDiscriminant);
			const double Q = fastCubeRootd((double)0.5 * TwoQcubed);
			const double TwoSsquared = (Q + delta0/Q - 2 * p)/(double)3.0;
#if DEBUG_QUARTIC
			debugPrintf("D0 D1, 2Q3, Q, 2s2 = %.15g  %.15g %.15g %.15g %.15g\n", delta0, delta1, TwoQcubed, Q, TwoSsquared);
#endif
			if (TwoSsquared < (double)0.0)
			{
				return 0;
			}
			const double S = (double)0.5 * sqrt(TwoSsquared);
			const double temp1 = -4 * dsquare(S) - 2 * p + q/S;
			const double temp2 = -4 * dsquare(S) - 2 * p - q/S;
#if DEBUG_QUARTIC
			debugPrintf("temp1,2 = %.7g %.7g\n", temp1, temp2);
#endif
			size_t numSolutions = 0;
			if (temp1 == (double)0.0)
			{
				rslt[numSolutions++] = -(double)0.25 * b - S;
			}
			else if (temp1 > (double)0.0)
			{
				rslt[numSolutions++] = -(double)0.25 * b - S - (double)0.5 * fastSqrtd(temp1);
				rslt[numSolutions++] = -(double)0.25 * b - S + (double)0.5 * fastSqrtd(temp1);
			}
			if (temp2 == (double)0.0)
			{
				rslt[numSolutions++] = -(double)0.25 * b + S;
			}
			else if (temp2 > (double)0.0)
			{
				rslt[numSolutions++] = -(double)0.25 * b + S - (double)0.5 * fastSqrtd(temp2);
				rslt[numSolutions++] = -(double)0.25 * b + S + (double)0.5 * fastSqrtd(temp2);
			}
			return SortRoots(rslt, numSolutions);
		}
	}
}

#endif

// End
