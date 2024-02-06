/*
 * RingBuffer.h
 *
 *  Created on: 13 Sep 2019
 *      Author: David
 */

#ifndef SRC_GENERAL_RINGBUFFER_H_
#define SRC_GENERAL_RINGBUFFER_H_

#include <ecv_duet3d.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <utility>

// Ring buffer template, used for serial I/O
// We assume the items are small (e.g. characters, floats) so we pass them by value in PutItem
// We use memcpy to copy them, so they must not have non-trivial copy constructors/assignment operators

template<class T> class RingBuffer
{
public:
	RingBuffer() noexcept;

	// Initialise and allocate the buffer. numSlots must be a power of 2.
	void Init(size_t numSlots) noexcept;

	// Store one item returning true if successful
	bool PutItem(T val) noexcept;

	// Store a block returning the number of items actually stored
	size_t PutBlock(const T* _ecv_array buffer, size_t buflen) noexcept;

	// Get one item returning true if successful
	bool GetItem(T& val) noexcept;

	// Get a block returning the number of items actually fetched
	size_t GetBlock(T* _ecv_array buffer, size_t buflen) noexcept;

	// Return the number of items we could currently add to the buffer
	size_t SpaceLeft() const noexcept;

	// Return the number of items in the buffer
	size_t ItemsPresent() const noexcept;

	// Return true if there are no items in the buffer
	bool IsEmpty() const noexcept;

	// Return the capacity
	size_t GetCapacity() const noexcept { return capacity; }

	// Clear the buffer
	void Clear() { getIndex = putIndex = 0; }

private:
	size_t capacity;			// must be one less than a power of 2

	// Declare the data volatile so that it can be accessed by multiple threads or ISRs (but max 1 getter and 1 putter concurrently)
	volatile size_t putIndex, getIndex;
	volatile T * _ecv_array _ecv_null data;
};

template<class T> RingBuffer<T>::RingBuffer() noexcept
	: capacity(0), putIndex(0), getIndex(0), data(nullptr)
{
}

template<class T> void RingBuffer<T>::Init(size_t numSlots) noexcept
{
	capacity = 0;
	putIndex = 0;
	getIndex = 0;
	volatile T *oldData = nullptr;
	std::swap(data, oldData);
	delete[] oldData;
	if (numSlots > 1)
	{
		capacity = numSlots - 1;
		data = new T[numSlots];
	}
}

template<class T> inline bool RingBuffer<T>::PutItem(T val) noexcept
{
	const size_t oldPutIndex = putIndex;				// capture volatile
	const size_t newPutIndex = (oldPutIndex + 1) & capacity;
	if (newPutIndex != getIndex)
	{
		not_null(data)[oldPutIndex] = val;
		putIndex = newPutIndex;
		return true;
	}
	return false;
}

template<class T> inline bool RingBuffer<T>::GetItem(T& val) noexcept
{
	const size_t currentGetIndex = getIndex;			// capture volatile
	if (currentGetIndex != putIndex)
	{
		val = not_null(data)[currentGetIndex];
		getIndex = (currentGetIndex + 1) & capacity;
		return true;
	}
	return false;
}

template<class T> inline size_t RingBuffer<T>::SpaceLeft() const noexcept
{
	return (getIndex + capacity - putIndex) & capacity;
}

template<class T> inline size_t RingBuffer<T>::ItemsPresent() const noexcept
{
	return (putIndex - getIndex) & capacity;
}

template<class T> inline bool RingBuffer<T>::IsEmpty() const noexcept
{
	return getIndex == putIndex;
}

template<class T> size_t RingBuffer<T>::PutBlock(const T* _ecv_array buffer, size_t buflen) noexcept
{
	// Capture volatile variables to avoid reloading them unnecessarily
	const size_t currentGetIndex = getIndex;
	size_t currentPutIndex = putIndex;

	// See how much room there is in the buffer and how much we can store
	size_t toCopy = (currentGetIndex + capacity - currentPutIndex) & capacity;
	if (buflen < toCopy)
	{
		toCopy = buflen;
	}

	if (toCopy != 0)
	{
		size_t toCopyNext;
		if (currentGetIndex <= currentPutIndex)
		{
			// Start by storing items from currentPutIndex up to the end of the buffer
			size_t toCopyFirst = capacity + 1 - currentPutIndex;
			if (toCopy < toCopyFirst)
			{
				// We don't reach the end of the buffer
				memcpy(const_cast<T* _ecv_array>(not_null(data)) + currentPutIndex, buffer, toCopy * sizeof(T));
				putIndex = currentPutIndex + toCopy;
				return toCopy;
			}
			memcpy(const_cast<T* _ecv_array>(not_null(data)) + currentPutIndex, buffer, toCopyFirst * sizeof(T));
			currentPutIndex = 0;
			toCopyNext = toCopy - toCopyFirst;
			buffer += toCopyFirst;
		}
		else
		{
			toCopyNext = toCopy;
		}
		memcpy(const_cast<T* _ecv_array>(not_null(data)) + currentPutIndex, buffer, toCopyNext * sizeof(T));
		putIndex = currentPutIndex + toCopyNext;
	}
	return toCopy;
}

template<class T> size_t RingBuffer<T>::GetBlock(T* _ecv_array buffer, size_t buflen) noexcept
{
	// Capture volatile variables to avoid reloading them unnecessarily
	size_t currentGetIndex = getIndex;
	const size_t currentPutIndex = putIndex;

	// See how much data there is in the buffer and how much we can fetch
	size_t toCopy = (currentPutIndex - currentGetIndex) & capacity;
	if (buflen < toCopy)
	{
		toCopy = buflen;
	}

	if (toCopy != 0)
	{
		size_t toCopyNext;
		if (currentGetIndex > currentPutIndex)
		{
			// Start by fetching items from currentGetIndex up to the end of the buffer
			const size_t toCopyFirst = capacity + 1 - currentGetIndex;
			if (toCopy < toCopyFirst)
			{
				// We don't reach the end of the buffer
				memcpy(buffer, const_cast<const T* _ecv_array>(not_null(data)) + currentGetIndex, toCopy * sizeof(T));
				getIndex = currentGetIndex + toCopy;
				return toCopy;
			}
			memcpy(buffer, const_cast<const T* _ecv_array>(not_null(data)) + currentGetIndex, toCopyFirst * sizeof(T));
			currentGetIndex = 0;
			toCopyNext = toCopy - toCopyFirst;
			buffer += toCopyFirst;
		}
		else
		{
			toCopyNext = toCopy;
		}
		memcpy(buffer, const_cast<const T* _ecv_array>(not_null(data)) + currentGetIndex, toCopyNext * sizeof(T));
		getIndex = currentGetIndex + toCopyNext;
	}
	return toCopy;
}

#endif /* SRC_GENERAL_RINGBUFFER_H_ */
