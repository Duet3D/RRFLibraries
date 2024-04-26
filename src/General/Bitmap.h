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
#include "function_ref.h"
#include "gcc_builtins.h"
#include "ecv_duet3d.h"

// Helper functions to work on bitmaps of various lengths.
// The primary purpose of these is to allow us to switch between 16, 32 and 64-bit bitmaps.

// Find the lowest set bit. Returns the lowest set bit number, undefined if no bits are set.
// GCC provides intrinsics, but unhelpfully they are in terms of int, long and long long instead of uint32_t, uint64_t etc.
inline unsigned int LowestSetBit(unsigned char val) noexcept
{
	return (unsigned int)__builtin_ctz(val);
}

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

// Extract one bit from a value and move it to a target bit number, returning a value with only the target bit possibly set
// T should be an unsigned integer type
template<class T> inline constexpr T ExtractBit(T val, unsigned int fromBitNumber, unsigned int toBitNumber) noexcept
{
	if (fromBitNumber == toBitNumber)
	{
		return val & ((T)1 << toBitNumber);
	}
	else if (toBitNumber > fromBitNumber)
	{
		return (val << (toBitNumber - fromBitNumber)) & ((T)1 << toBitNumber);
	}
	else
	{
		return (val >> (fromBitNumber - toBitNumber)) & ((T)1 << toBitNumber);
	}
}

// Extract two bits from a value and move it to a target bit number, returning a value with only the target bits possibly set
// T should be an unsigned integer type
template<class T> inline constexpr T ExtractTwoBits(T val, unsigned int fromBitNumber, unsigned int toBitNumber) noexcept
{
	if (fromBitNumber == toBitNumber)
	{
		return val & ((T)3 << toBitNumber);
	}
	else if (toBitNumber > fromBitNumber)
	{
		return (val << (toBitNumber - fromBitNumber)) & ((T)3 << toBitNumber);
	}
	else
	{
		return (val >> (fromBitNumber - toBitNumber)) & ((T)3 << toBitNumber);
	}
}

template<class BaseType> class Bitmap
{
public:
	constexpr Bitmap() noexcept : bits(0) { }
	explicit constexpr Bitmap(BaseType n) noexcept : bits(n) { }

	static constexpr unsigned int MaxBits() noexcept { return sizeof(BaseType) * CHAR_BIT; }
	constexpr BaseType GetRaw() const noexcept { return bits; }
	constexpr bool IsEmpty() const noexcept { return bits == 0; }
	constexpr bool IsNonEmpty() const noexcept { return bits != 0; }

	constexpr bool IsBitSet(unsigned int n) const noexcept
	{
		return (bits & ((BaseType)1u << n)) != 0;
	}

	constexpr bool IsAnyBitSet(unsigned int n1, unsigned int n2) const noexcept
	{
		return (bits & (((BaseType)1u << n1) | ((BaseType)1u << n2))) != 0;
	}

	constexpr bool IsAnyBitSet(unsigned int n1, unsigned int n2, unsigned int n3) const noexcept
	{
		return (bits & (((BaseType)1u << n1) | ((BaseType)1u << n2) | ((BaseType)1u << n3))) != 0;
	}

	constexpr void Clear() noexcept { bits = 0; }

	// Set a bit
	constexpr void SetBit(unsigned int n) noexcept
	{
		bits |= ((BaseType)1u << n);
	}

	// Clear a bit
	constexpr void ClearBit(unsigned int n) noexcept
	{
		bits &= ~((BaseType)1u << n);
	}

	// Invert a bit
	constexpr void InvertBit(unsigned int n) noexcept
	{
		bits ^= ((BaseType)1u << n);
	}

	// Clear multiple bits
	constexpr void ClearBits(Bitmap<BaseType> other) noexcept
	{
		bits &= ~other.bits;
	}

	// Set a bit to a given state
	constexpr void SetOrClearBit(unsigned int n, bool b) noexcept
	{
		if (b)
		{
			bits |= ((BaseType)1u << n);
		}
		else
		{
			bits &= ~((BaseType)1u << n);
		}
	}

	constexpr Bitmap<BaseType> operator & (Bitmap<BaseType> other) const noexcept
	{
		return Bitmap<BaseType>(bits & other.bits);
	}

	constexpr Bitmap<BaseType>& operator &= (Bitmap<BaseType> other) noexcept
	{
		bits &= other.bits;
		return *this;
	}

	constexpr Bitmap<BaseType> operator | (Bitmap<BaseType> other) const noexcept
	{
		return Bitmap<BaseType>(bits | other.bits);
	}

	constexpr Bitmap<BaseType> operator ~ () const noexcept
	{
		return Bitmap<BaseType>(~bits);
	}

	constexpr Bitmap<BaseType>& operator |= (Bitmap<BaseType> other) noexcept
	{
		bits |= other.bits;
		return *this;
	}

	constexpr Bitmap<BaseType> operator - (Bitmap<BaseType> other) const noexcept
	{
		return Bitmap<BaseType>(bits & ~other.bits);
	}

	constexpr bool operator == (Bitmap<BaseType> other) const noexcept
	{
		return bits == other.bits;
	}

	constexpr bool operator != (Bitmap<BaseType> other) const noexcept
	{
		return bits != other.bits;
	}

	constexpr bool Intersects(Bitmap<BaseType> other) const noexcept
	{
		return (bits & other.bits) != 0;
	}

	constexpr bool Disjoint(Bitmap<BaseType> other) const noexcept
	{
		return (bits & other.bits) == 0;
	}

	constexpr bool Contains(Bitmap<BaseType> other) const noexcept
	{
		return (~bits & other.bits) == 0;
	}

	constexpr Bitmap<BaseType> ShiftUp(unsigned int n) const noexcept
	{
		return Bitmap<BaseType>(bits << n);
	}

	// Set the bits from a base type bitmap
	constexpr void SetFromRaw(BaseType b) noexcept
	{
		bits = b;
	}

	// Count the number of set bits
	unsigned int CountSetBits() const noexcept;

	// Get the position of the Nth set bit, or -1 if too few bits set
	int GetSetBitNumber(size_t index) const noexcept;

	unsigned int LowestSetBit() const noexcept
	{
		return ::LowestSetBit(bits);
	}

	// Iterate over the bits
	void Iterate(function_ref_noexcept<void(unsigned int, unsigned int) noexcept> func) const noexcept;
	void IterateWithExceptions(function_ref<void(unsigned int, unsigned int)> func) const;
	bool IterateWhile(function_ref_noexcept<bool(unsigned int, unsigned int) noexcept> func) const noexcept;

	// Make a bitmap with the lowest n bits set
	static constexpr Bitmap<BaseType> MakeLowestNBits(unsigned int n) noexcept
	{
		return Bitmap<BaseType>((n < MaxBits()) ? ((BaseType)1u << n) - 1 : ~(BaseType)0);
	}

	// Convert an unsigned integer to a bit in a bitmap
	static constexpr Bitmap<BaseType> MakeFromBits(unsigned int b1) noexcept
	{
		return Bitmap<BaseType>((BaseType)1u << b1);
	}

	// Convert an unsigned integer to a bit in a bitmap
	static constexpr Bitmap<BaseType> MakeFromBits(unsigned int b1, unsigned int b2) noexcept
	{
		return Bitmap<BaseType>((BaseType)1u << b1 | (BaseType)1u << b2);
	}

	// Convert an unsigned integer to a bit in a bitmap
	static constexpr Bitmap<BaseType> MakeFromBits(unsigned int b1, unsigned int b2, unsigned int b3) noexcept
	{
		return Bitmap<BaseType>((BaseType)1u << b1 | (BaseType)1u << b2 |  (BaseType)1u << b3);
	}

	// Convert an unsigned integer to a bit in a bitmap
	static constexpr Bitmap<BaseType> MakeFromRaw(BaseType b) noexcept
	{
		return Bitmap<BaseType>(b);
	}

	// Convert an array of unsigned longs to a bit map with overflow checking
	static Bitmap<BaseType> MakeFromArray(const uint32_t *_ecv_array arr, size_t numEntries) noexcept;

	// Convert an array of longs to a bit map with overflow checking
	static Bitmap<BaseType> MakeFromArray(const int32_t *_ecv_array arr, size_t numEntries) noexcept;

private:
	static constexpr uint8_t BitCount[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

	BaseType bits;
};

// Count the number of set bits
template<class BaseType> unsigned int Bitmap<BaseType>::CountSetBits() const noexcept
{
	unsigned int count = 0;
	BaseType b = bits;
	while (b != 0)
	{
		count += BitCount[b & 0x0F];
		b >>= 4;
	}
	return count;
}

// Get the position of the Nth set bit, or -1 if too few bits set
template<class BaseType> int Bitmap<BaseType>::GetSetBitNumber(size_t index) const noexcept
{
	BaseType b = bits;
	while (b != 0)
	{
		const unsigned int lowestBit = ::LowestSetBit(b);
		if (index == 0)
		{
			return (int)lowestBit;
		}
		b &= ~((BaseType)1u << lowestBit);
		--index;
	}
	return -1;
}

// Iterate over the bits
template<class BaseType> void Bitmap<BaseType>::Iterate(function_ref_noexcept<void(unsigned int, unsigned int) noexcept > func) const noexcept
{
	BaseType copyBits = bits;
	unsigned int count = 0;
	while (copyBits != 0)
	{
		const unsigned int index = ::LowestSetBit(copyBits);
		func(index, count);
		copyBits &= ~((BaseType)1 << index);
		++count;
	}
}

// Iterate over the bits
template<class BaseType> void Bitmap<BaseType>::IterateWithExceptions(function_ref<void(unsigned int, unsigned int)> func) const
{
	BaseType copyBits = bits;
	unsigned int count = 0;
	while (copyBits != 0)
	{
		const unsigned int index = ::LowestSetBit(copyBits);
		func(index, count);
		copyBits &= ~((BaseType)1 << index);
		++count;
	}
}

// Iterate over the bits while the passed function returns true. Return true if we reached the end, false if we terminated because the passed function returned false.
template<class BaseType> bool Bitmap<BaseType>::IterateWhile(function_ref_noexcept<bool(unsigned int, unsigned int) noexcept > func) const noexcept
{
	BaseType copyBits = bits;
	unsigned int count = 0;
	while (copyBits != 0)
	{
		const unsigned int index = ::LowestSetBit(copyBits);
		if (!func(index, count))
		{
			return false;
		}
		copyBits &= ~((BaseType)1 << index);
		++count;
	}
	return true;
}

// Convert an array of unsigned longs to a bit map with overflow checking
template<class BaseType> Bitmap<BaseType> Bitmap<BaseType>::MakeFromArray(const uint32_t *_ecv_array arr, size_t numEntries) noexcept
{
	BaseType res = 0;
	for (size_t i = 0; i < numEntries; ++i)
	{
		const uint32_t f = arr[i];
		if (f < Bitmap<BaseType>::MaxBits())
		{
			res |= (BaseType)1 << f;
		}
	}
	return Bitmap<BaseType>(res);
}

// Convert an array of longs to a bit map with overflow checking
template<class BaseType> Bitmap<BaseType> Bitmap<BaseType>::MakeFromArray(const int32_t *_ecv_array arr, size_t numEntries) noexcept
{
	BaseType res = 0;
	for (size_t i = 0; i < numEntries; ++i)
	{
		const int32_t f = arr[i];
		if (f >= 0 && f < (int32_t)Bitmap<BaseType>::MaxBits())
		{
			res |= (BaseType)1 << (unsigned int)f;
		}
	}
	return Bitmap<BaseType>(res);
}

// Class to hold a bitmap that won't fit into a single object f integral type
template<unsigned int N> class LargeBitmap
{
public:
	LargeBitmap() noexcept { ClearAll(); }

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
	static constexpr size_t numDwords = (N + 31)/32;

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
