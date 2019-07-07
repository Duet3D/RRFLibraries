/*
 * Bitmap.h
 *
 *  Created on: 5 Jul 2019
 *      Author: David
 */

#ifndef SRC_GENERAL_BITMAP_H_
#define SRC_GENERAL_BITMAP_H_

#include <cstdint>
#include <cstddef>
#include <climits>

// Helper functions to work on bitmaps of various lengths.
// The primary purpose of these is to allow us to switch between 16, 32 and 64-bit bitmaps.

// Convert an unsigned integer to a bit in a bitmap
template<typename BitmapType> inline constexpr BitmapType MakeBitmap(unsigned int n)
{
	return (BitmapType)1u << n;
}

// Make a bitmap with the lowest n bits set
template<typename BitmapType> inline constexpr BitmapType LowestNBits(unsigned int n)
{
	return ((BitmapType)1u << n) - 1;
}

// Check if a particular bit is set in a bitmap
template<typename BitmapType> inline constexpr bool IsBitSet(BitmapType b, unsigned int n)
{
	return (b & ((BitmapType)1u << n)) != 0;
}

// Set a bit in a bitmap
template<typename BitmapType> inline void SetBit(BitmapType &b, unsigned int n)
{
	b |= ((BitmapType)1u << n);
}

// Clear a bit in a bitmap
template<typename BitmapType> inline void ClearBit(BitmapType &b, unsigned int n)
{
	b &= ~((BitmapType)1u << n);
}

// Convert an array of longs to a bit map with overflow checking
template<typename BitmapType> BitmapType UnsignedArrayToBitMap(const uint32_t *arr, size_t numEntries)
{
	BitmapType res = 0;
	for (size_t i = 0; i < numEntries; ++i)
	{
		const uint32_t f = arr[i];
		if (f < sizeof(BitmapType) * CHAR_BIT)
		{
			SetBit(res, f);
		}
	}
	return res;
}

#endif /* SRC_GENERAL_BITMAP_H_ */
