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
template<typename BitmapType> inline constexpr BitmapType MakeBitmap(unsigned int n) noexcept
{
	return (BitmapType)1u << n;
}

// Make a bitmap with the lowest n bits set
template<typename BitmapType> inline constexpr BitmapType LowestNBits(unsigned int n) noexcept
{
	return ((BitmapType)1u << n) - 1;
}

// Check if a particular bit is set in a bitmap
template<typename BitmapType> inline constexpr bool IsBitSet(BitmapType b, unsigned int n) noexcept
{
	return (b & ((BitmapType)1u << n)) != 0;
}

// Set a bit in a bitmap
template<typename BitmapType> inline void SetBit(BitmapType &b, unsigned int n) noexcept
{
	b |= ((BitmapType)1u << n);
}

// Clear a bit in a bitmap
template<typename BitmapType> inline void ClearBit(BitmapType &b, unsigned int n) noexcept
{
	b &= ~((BitmapType)1u << n);
}

// Convert an array of longs to a bit map with overflow checking
template<typename BitmapType> BitmapType UnsignedArrayToBitMap(const uint32_t *arr, size_t numEntries) noexcept
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

// Find the lowest set bit. Returns the lowest set bit number, undefined if no bits are set.
// GCC provides intrinsics, but unhelpfully they are in terms of int, long and long long instead of uint32_t, uint64_t etc.
inline unsigned int LowestSetBit(unsigned short int val) noexcept
{
	return (unsigned int)__builtin_ctz(val);
}

inline unsigned int LowestSetBit(unsigned int val) noexcept
{
	return (unsigned int)__builtin_ctz(val);
}

inline unsigned int LowestSetBit(unsigned long val) noexcept
{
	return (unsigned int)__builtin_ctzl(val);
}

inline unsigned int LowestSetBit(unsigned long long val) noexcept
{
	return (unsigned int)__builtin_ctzll(val);
}

// Class to hold a bitmap that won't fit into a single object f integral type
template<unsigned int N> class LargeBitmap
{
public:
	void ClearAll() noexcept;

	void SetBit(unsigned int n) noexcept
	{
		if (n < N)
		{
			data[n >> 5] |= (1ul << (n & 31));
		}
	}

	void ClearBit(unsigned int n) noexcept
	{
		if (n < N)
		{
			data[n >> 5] &= ~(1ul << (n & 31));
		}
	}

	bool IsBitSet(unsigned int n) const noexcept
	{
		return n < N && (data[n >> 5] & (1ul << (n & 31))) != 0;
	}

	unsigned int FindLowestSetBit() const noexcept;

	static constexpr unsigned int NumBits() noexcept { return N; }

private:
	static constexpr size_t numDwords = (N + 31/32);

	uint32_t data[numDwords];
};

template<unsigned int N> void LargeBitmap<N>::ClearAll() noexcept
{
	for (uint32_t& v : data)
	{
		v = 0;
	}
}

template<unsigned int N> unsigned int LargeBitmap<N>::FindLowestSetBit() const noexcept
{
	for (unsigned int i = 0; i < numDwords; ++i)
	{
		if (data[i] != 0)
		{
			return (i << 5) + LowestSetBit(data[i]);
		}
	}
	return N;
}

#endif /* SRC_GENERAL_BITMAP_H_ */
