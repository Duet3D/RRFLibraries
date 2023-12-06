/*
 * NumericConverter.cpp
 *
 *  Created on: 26 Apr 2020
 *      Author: David
 */

#include "NumericConverter.h"
#include "SimpleMath.h"					// for ARRAY_SIZE
#include <cmath>
#include <cctype>
#include <cstddef>

#ifdef __ECV__	// eCv doesn't support numeric_limits yet
constexpr uint32_t Uint32Max = 0xffffffffu;
constexpr int32_t Int32Max = 0x7fffffff;
constexpr int32_t Int32Min = -Int32Max - 1;
#else
# include <limits>
constexpr uint32_t Uint32Max = std::numeric_limits<uint32_t>::max();
constexpr int32_t Int32Max= std::numeric_limits<int32_t>::max();
constexpr int32_t Int32Min = std::numeric_limits<int32_t>::min();
#endif

// Function to read an unsigned integer or real literal and store the values in this object
// On entry, 'c' is the first character to consume and NextChar is the function to get another character
// Returns true if a valid number was found. If it returns false then characters may have been consumed.
// On return the value parsed is: lvalue * 2^twos * 5^fives
bool NumericConverter::Accumulate(char c, OptionsType options, function_ref_noexcept<char() noexcept> NextChar) noexcept
{
	hadDecimalPoint = hadExponent = isNegative = false;
	bool hadDigit = false;
	unsigned int base = ((options & DefaultHex) != 0) ? 16 : 10;
	lvalue = 0;
	fives = twos = 0;

	// 1. Skip white space
	while (c == ' ' || c == '\t')
	{
		c = NextChar();
	}

	// 2. Check for a sign
	if (c == '+')
	{
		c = NextChar();
	}
	else if (c == '-')
	{
		if ((options & AcceptNegative) == 0)
		{
			return false;
		}
		isNegative = true;
		c = NextChar();
	}

	// If hex allowed, check for leading 0x
	if (c == '0' && (options & AcceptHex) != 0)
	{
		hadDigit = true;
		c = NextChar();
		if ((char)toupper(c) == 'X')
		{
			base = 16;
			options &= ~AcceptFloat;
			c = NextChar();
		}
		else if ((char)toupper(c) == 'B')
		{
			base = 2;
			options &= ~AcceptFloat;
			c = NextChar();
		}
	}

	// Skip leading zeros, but count the number of them after the decimal point
	for (;;)
	{
		if (c == '0')
		{
			hadDigit = true;
			if (hadDecimalPoint)
			{
				--fives;
				--twos;
			}
		}
		else if (c == '.' && !hadDecimalPoint && (options & AcceptFloat) != 0)
		{
			hadDecimalPoint = true;
		}
		else
		{
			break;
		}
		c = NextChar();
	}

	// Read digits and allow a decimal point if we haven't already had one
	bool overflowed = false;
	for (;;)
	{
		if ((bool)isxdigit(c))
		{
			const unsigned int digit = (c <= '9') ? c - '0' : (toupper(c) - (int)'A') + 10;
			if (digit >= base)
			{
				break;
			}

			hadDigit = true;

			switch (base)
			{
			case 2:
				if (overflowed)
				{
					++twos;
				}
				else if (lvalue <= Uint32Max/2)
				{
					lvalue = (lvalue << 1u) + digit;
				}
				else
				{
					overflowed = true;
					++twos;
				}
				break;

			case 10:
				if (overflowed)
				{
					if (!hadDecimalPoint)
					{
						++fives;
						++twos;
					}
				}
				else if (   lvalue <= (Uint32Max - 9u)/10u		// avoid slow division if we can
						 || lvalue <= (Uint32Max - digit)/10u
					    )
				{
					lvalue = (lvalue * 10u) + digit;
					if (hadDecimalPoint)
					{
						--fives;
						--twos;
					}
				}
				else
				{
					overflowed = true;
					const unsigned int fivesDigit = (digit + 1u)/2u;
					if (lvalue <= (Uint32Max - fivesDigit)/5u)
					{
						lvalue = (lvalue * 5u) + fivesDigit;
						if (hadDecimalPoint)
						{
							--fives;
						}
						else
						{
							++twos;
						}
					}
					else
					{
						const unsigned int twosDigit = (digit + 4u)/5u;
						if (lvalue <= (Uint32Max - twosDigit)/2u)
						{
							lvalue = (lvalue * 2u) + twosDigit;
							if (hadDecimalPoint)
							{
								--twos;
							}
							else
							{
								++fives;
							}
						}
						else if (!hadDecimalPoint)
						{
							++fives;
							++twos;
						}
					}
				}
				break;

			case 16:
				if (overflowed)
				{
					twos += 4;
				}
				else if (lvalue <= Uint32Max/16)
				{
					lvalue = (lvalue << 4u) + digit;
				}
				else
				{
					overflowed = true;
					if (lvalue <= Uint32Max/8)
					{
						lvalue = (lvalue << 3u) | ((digit + 1u) >> 1u);
						++twos;
					}
					else if (lvalue <= Uint32Max/4)
					{
						lvalue = (lvalue << 2u) | ((digit + 2u) >> 2u);
						twos += 2;
					}
					else if (lvalue <= Uint32Max/2)
					{
						lvalue = (lvalue << 1u) | ((digit + 4u) >> 3u);
						twos += 3;
					}
					else
					{
						twos += 4;
					}
				}
				break;
			}
		}
		else if (c == '.' && !hadDecimalPoint && (options & AcceptFloat) != 0)
		{
			hadDecimalPoint = true;
		}
		else
		{
			break;
		}
		c = NextChar();
	}

	if (!hadDigit)
	{
		return false;
	}

	// Check for an exponent
	if ((options & AcceptFloat) != 0 && (char)toupper(c) == 'E')
	{
		c = NextChar();

		// 5a. Check for signed exponent
		const bool expNegative = (c == '-');
		if (expNegative || c == '+')
		{
			c = NextChar();
		}

		if (!(bool)isdigit(c))
		{
			return false;								// E or e not followed by a number
		}

		// 5b. Read exponent digits
		hadExponent = true;
		unsigned int exponent = 0;
		while ((bool)isdigit(c))
		{
			// Limit the exponent range to avoid overflow.
			// Exponent range for a float is -126..+127 but to allow for denormalised numbers we must support down to about -150.
			// If we ever add a GetDouble function then we will need to allow about -1060 to +1023.
			if (exponent < 160)
			{
				exponent = (10u * exponent) + (unsigned int)(c - '0');
			}
			c = NextChar();
		}

		if (expNegative)
		{
			twos -= (int)exponent;
			fives -= (int)exponent;
		}
		else
		{
			twos += (int)exponent;
			fives += (int)exponent;
		}
	}

	return true;
}

// Return true if the number fits in an int32 and wasn't specified with a decimal point or an exponent
// Note, we don't allow the value to be the most negative int32_t available
bool NumericConverter::FitsInInt32() const noexcept
{
	return !hadDecimalPoint && !hadExponent && twos == 0 && fives == 0 && lvalue <= (uint32_t)Int32Max;
}

// Return true if the number fits in a uint32 and wasn't specified with a decimal point or an exponent
bool NumericConverter::FitsInUint32() const noexcept
{
	return !hadDecimalPoint && !hadExponent && (!isNegative || lvalue == 0) && twos == 0 && fives == 0;
}

// Given that FitsInInt32() returns true, return the number as an int32_t
int32_t NumericConverter::GetInt32() const noexcept
{
	return (isNegative) ? -(int32_t)lvalue : (int32_t)lvalue;
}

// Given that FitsInUint32() returns true, return the number as a uint32_t
uint32_t NumericConverter::GetUint32() const noexcept
{
	return lvalue;
}

// We need a table of double-precision constants, but we compile with -fsingle-precision-constant.
// This macro lets us use a double precision constant, by declaring it as a long double one and then casting it to double.
#define DOUBLE(_x) ((double)( _x ## L ))

static constexpr double PowersOfTen[] =
{
	DOUBLE(1.0),
	DOUBLE(10.0),
	DOUBLE(100.0),
	DOUBLE(1000.0),
	DOUBLE(10000.0),
	DOUBLE(100000.0),
	DOUBLE(1000000.0),
	DOUBLE(10000000.0),
	DOUBLE(100000000.0),
	DOUBLE(1000000000.0),
	DOUBLE(10000000000.0)
};

// Return the value as a float
float NumericConverter::GetFloat() const noexcept
{
	// The pow() function includes 4K of logarithm tables, so avoid using it
	double dvalue = (double)lvalue;
	{
		int tens = (twos < fives) ? twos : fives;
		while (tens < 0 && dvalue != DOUBLE(0.0))
		{
			const size_t power = min<size_t>((size_t)-tens, ARRAY_SIZE(PowersOfTen) - 1);
			dvalue /= PowersOfTen[power];
			tens += (int)power;
		}
		while (tens > 0 && !std::isinf(dvalue))
		{
			const size_t power = min<size_t>((size_t)tens, ARRAY_SIZE(PowersOfTen) - 1);
			dvalue *= PowersOfTen[power];
			tens -= (int)power;
		}
	}

	// Fives may be one greater than twos if the base was 10, and twos may be many more than fives
	if (fives > twos)
	{
		dvalue *= 5;
	}
	else
	{
		for (int n = fives; n < twos && !std::isinf(dvalue); ++n)
		{
			dvalue *= 2;
		}
	}

	return (isNegative) ? -(float)dvalue : (float)dvalue;
}

// Get the number of decimal digits that might be worth displaying after the decimal point when we print this.
// the caller must limit the return value to a sensible value for the float or double type used.
unsigned int NumericConverter::GetDigitsAfterPoint() const noexcept
{
	const int digits = (fives < twos) ? fives : twos;
	return (digits < 0) ? (unsigned int)-digits : 0;
}

// End
