/*
 * Power.cpp
 *
 *  Created on: 25 Apr 2020
 *      Author: David
 */

#include "Power.h"
#include <cmath>

// We need a table of double-precision constants, but we compile with -fsingle-precision-constant.
// This macro lets us use a double precision constant, by declaring it as a long double one and then casting it to double.
#define DOUBLE(_x) ((double)( _x ## L ))

static constexpr double inversePowersOfTen[] =
{
	DOUBLE(0.1),
	DOUBLE(0.01),
	DOUBLE(0.001),
	DOUBLE(0.0001),
	DOUBLE(0.00001),
	DOUBLE(0.000001),
	DOUBLE(0.0000001),
	DOUBLE(0.00000001),
	DOUBLE(0.000000001),
	DOUBLE(0.0000000001),
	DOUBLE(0.00000000001),
	DOUBLE(0.000000000001)
};

// Function to raise a double to a power of 10, optimising the common cases used by SafeStrtod
double TimesPowerOf10(double d, long exp)
{
	if (exp == 0)
	{
		return d;
	}
	if (exp < 0 && exp >= -(long)(sizeof(inversePowersOfTen)/sizeof(inversePowersOfTen[0])))
	{
		return d * inversePowersOfTen[-exp - 1];
	}
	return d * pow(DOUBLE(10.0), (double)exp);
}

// End
