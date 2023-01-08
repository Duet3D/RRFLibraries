/*
 * strtod.cpp
 *
 *  Created on: 4 Apr 2018
 *      Author: David
 *
 * This is a replacement for strtof() in the C standard library. That version has two problems:
 * 1. It is not reentrant. We can make it so by defining configUSE_NEWLIB_REENTRANT in FreeRTOS, but that makes the tasks much bigger.
 * 2. It allocates and releases heap memory, which is not nice.
 *
 * Limitations of this versions
 * 1. Rounding to nearest float might possibly not always be correct.
 * 2. Does not handle overflow for stupidly large numbers correctly.
 */

#include <cstdint>
#include <cctype>
#include <cmath>
#include <cstdlib>

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

#include "SafeStrtod.h"
#undef strtoul		// Undo the macro definition of strtoul in SafeStrtod.h so that we can call it in this file

#include "NumericConverter.h"

float SafeStrtof(const char *_ecv_array s, const char *_ecv_array *null endptr) noexcept
{
	// Save the end pointer in case of failure
	if (endptr != nullptr)
	{
		*not_null(endptr) = s;
	}

	// Parse the number
	NumericConverter conv;
	if (conv.Accumulate(*s, NumericConverter::AcceptSignedFloat, [&s]()->char { ++s; return *s; } ))
	{
		if (endptr != nullptr)
		{
			*not_null(endptr) = s;
		}
		return conv.GetFloat();
	}

	return 0.0f;
}

static uint32_t StrToU32Opt(const char *_ecv_array s, const char *_ecv_array *null endptr, NumericConverter::OptionsType options) noexcept
{
	// Save the end pointer in case of failure
	if (endptr != nullptr)
	{
		*not_null(endptr) = s;
	}

	// Parse the number
	NumericConverter conv;
	if (conv.Accumulate(*s, options, [&s]()->char { ++s; return *s; } ))
	{
		if (endptr != nullptr)
		{
			*not_null(endptr) = s;
		}
		return (conv.FitsInUint32()) ? conv.GetUint32() : Uint32Max;
	}

	return 0;
}

uint32_t StrToU32(const char *_ecv_array s, const char *_ecv_array *null endptr) noexcept
{
	return StrToU32Opt(s, endptr, NumericConverter::AcceptOnlyUnsignedDecimal);
}

uint32_t StrOptHexToU32(const char *_ecv_array s, const char *_ecv_array *null endptr) noexcept
{
	return StrToU32Opt(s, endptr, NumericConverter::AcceptHex);
}

uint32_t StrHexToU32(const char *_ecv_array s, const char *_ecv_array *null endptr) noexcept
{
	return StrToU32Opt(s, endptr, NumericConverter::DefaultHex);
}

int32_t StrToI32(const char *_ecv_array s, const char *_ecv_array *null endptr) noexcept
{
	// Save the end pointer in case of failure
	if (endptr != nullptr)
	{
		*not_null(endptr) = s;
	}

	// Parse the number
	NumericConverter conv;
	if (conv.Accumulate(*s, NumericConverter::AcceptNegative, [&s]()->char { ++s; return *s; } ))
	{
		if (endptr != nullptr)
		{
			*not_null(endptr) = s;
		}
		return (conv.FitsInInt32()) ? conv.GetInt32()
				: (conv.IsNegative()) ? Int32Min
					: Int32Max;
	}

	return 0;
}

// End
