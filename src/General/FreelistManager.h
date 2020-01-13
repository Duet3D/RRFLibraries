/*
 * FreeListManager.h
 *
 *  Created on: 7 May 2018
 *      Author: David
 *
 *  Templated class to manage free lists, a different one for each object size
 */

#ifndef SRC_LIBRARIES_GENERAL_FREELISTMANAGER_H_
#define SRC_LIBRARIES_GENERAL_FREELISTMANAGER_H_

#include <cstddef>
#include "../RTOSIface/RTOSIface.h"

namespace FreelistManager
{
	// Free list manager class
	template<size_t Sz> class Freelist
	{
	public:
		static void *Allocate() noexcept;
		static void Release(void *p) noexcept;

	private:
		static void *freelist;
	};

	template<size_t Sz> void *Freelist<Sz>::freelist = nullptr;

	template<size_t Sz> void *Freelist<Sz>::Allocate() noexcept
	{
		TaskCriticalSectionLocker lock;

		if (freelist != nullptr)
		{
			void * const p = freelist;
			freelist = *static_cast<void **>(p);
			return p;
		}
		return ::operator new(Sz);
	}

	template<size_t Sz> void Freelist<Sz>::Release(void *p) noexcept
	{
		TaskCriticalSectionLocker lock;

		*static_cast<void **>(p) = freelist;
		freelist = p;
	}

	// Macro to return the size of objects of a given type rounded up to a multiple of 8 bytes.
	// We use this to reduce the number of freelists that we need to keep.
	inline constexpr size_t RoundedUpSize(size_t rawSize)
	{
		constexpr size_t sizeIncrement = 8;
		return ((rawSize + (sizeIncrement - 1u)) & ~(sizeIncrement - 1u));
	}

	// Operators new and delete for the classes that we want to use these freelists for should call the following functions
	template<class T> inline void *Allocate() noexcept
	{
		return Freelist<RoundedUpSize(sizeof(T))>::Allocate();
	}

	template<class T> inline void Release(void *p) noexcept
	{
		Freelist<RoundedUpSize(sizeof(T))>::Release(p);
	}
}

#endif /* SRC_LIBRARIES_GENERAL_FREELISTMANAGER_H_ */
