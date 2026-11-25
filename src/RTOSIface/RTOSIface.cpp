/*
 * RTOS.cpp
 *
 *  Created on: 30 Mar 2018
 *      Author: David
 */

#include "RTOSIface.h"
#include "RTOSNotifyIndices.h"

#ifdef RTOS

# include "../General/FreelistManager.h"
# include "FreeRTOS.h"
# include "task.h"
# include "semphr.h"
# include <atomic>

extern "C" [[noreturn]] void vAssertCalled(uint32_t line, c_string file) noexcept;
#define RTOS_ASSERT(_expr)	if (!(_expr)) { vAssertCalled(__LINE__, __FILE__); }

static_assert(Mutex::TimeoutUnlimited == portMAX_DELAY, "Bad value for TimeoutUnlimited");

// Create the semaphore. A semaphore must only be created once. The name is not copied, so pName must point to read-only or persistent storage.
void Mutex::Create(c_string pName) noexcept
{
	xSemaphoreCreateRecursiveMutexStatic(this);
	name = pName;
	next = mutexList;
	mutexList = this;
}

Mutex::~Mutex()
{
	// Unlink this mutex from the mutex list
	TaskCriticalSectionLocker lock;
	for (Mutex *_ecv_null *mpp = &mutexList; *mpp != nullptr; mpp = &not_null(*mpp)->next)
	{
		if (*mpp == this)
		{
			*mpp = not_null(*mpp)->next;
			break;
		}
	}
	vSemaphoreDelete(this);
}

// Take ownership of a mutex returning true if successful, false if timed out
bool Mutex::Take(uint32_t timeout) noexcept
{
	return xSemaphoreTakeRecursive(GetHandle(), timeout) == pdTRUE;
}

// Release a mutex returning true if successful.
// Note that the return value does not indicate whether the mutex is still owned, because it may have been taken more than once.
bool Mutex::Release() noexcept
{
	return xSemaphoreGiveRecursive(GetHandle()) == pdTRUE;
}

TaskHandle _ecv_null Mutex::GetHolder() const noexcept
{
	return reinterpret_cast<TaskBase *_ecv_null>(xSemaphoreGetMutexHolder(GetConstHandle()));
}

TaskBase *_ecv_from null TaskBase::taskList = nullptr;
TaskBase::TaskId TaskBase::numTasks = 0;

Mutex *null Mutex::mutexList = nullptr;

#else

volatile unsigned int RTOSIface::interruptCriticalSectionNesting = 0;

void Mutex::Create(c_string pName) noexcept
{
}

bool Mutex::Take(uint32_t timeout) noexcept
{
	return true;
}

bool Mutex::Release() noexcept
{
	return true;
}

TaskHandle Mutex::GetHolder() const noexcept
{
	return nullptr;
}

#endif

MutexLocker::MutexLocker(Mutex *null m, uint32_t timeout) noexcept
{
	handle = m;
	acquired =
#ifdef RTOS
				(m == nullptr) || not_null(m)->Take(timeout);
#else
				true;
#endif
}

MutexLocker::MutexLocker(Mutex& m, uint32_t timeout) noexcept
{
	handle = &m;
	acquired =
#ifdef RTOS
				m.Take(timeout);
#else
				true;
#endif
}

// Release the lock early (non-counting)
void MutexLocker::Release() noexcept
{
#ifdef RTOS
	if (acquired)
	{
		if (handle != nullptr)
		{
			not_null(handle)->Release();
		}
		acquired = false;
	}
#else
	acquired = false;
#endif
}

// Acquire it again, if it isn't already owned (non-counting)
bool MutexLocker::ReAcquire(uint32_t timeout) noexcept
{
#ifdef RTOS
	if (!acquired)
	{
		acquired = (handle == nullptr) || not_null(handle)->Take(timeout);
	}
#else
	acquired = true;
#endif
	return acquired;
}

#ifdef RTOS

BinarySemaphore::BinarySemaphore() noexcept
{
	 xSemaphoreCreateBinaryStatic(this);
}

bool BinarySemaphore::Take(uint32_t timeout) noexcept
{
	return xSemaphoreTake(GetHandle(), timeout);
}

bool BinarySemaphore::Give() noexcept
{
	return xSemaphoreGive(GetHandle());
}

// Link the task into the thread list and allocate a short task ID to it. Task IDs start at 1.
void TaskBase::AddToList() noexcept
{
	TaskCriticalSectionLocker lock;

	++numTasks;
	taskId = numTasks;
	next = taskList;
	taskList = this;
}

// Terminate a task and remove it from the thread list
void TaskBase::TerminateAndUnlink() noexcept
{
	if (taskId != 0)
	{
		taskId = 0;

		// Unlink the task from the thread list
		{
			TaskCriticalSectionLocker lock;
			for (TaskBase *_ecv_from null * tpp = &taskList; *tpp != nullptr; tpp = &not_null(*tpp)->next)
			{
				if (*tpp == this)
				{
					*tpp = not_null(*tpp)->next;
					break;
				}
			}
		}

		// Delete the task last in case it is deleting itself
		vTaskDelete(GetFreeRTOSHandle());
	}
}

// Wake up this task from an ISR
void TaskBase::GiveFromISR(uint32_t index) noexcept
{
	if (taskId != 0)			// check that the task has been created and not terminated
	{
		BaseType_t higherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveIndexedFromISR(GetFreeRTOSHandle(), index, &higherPriorityTaskWoken);
		portYIELD_FROM_ISR(higherPriorityTaskWoken);
	}
}

#endif

// Class ReadWriteLock members
// Each element in the readLocks list is a lock if the count is nonzero or a request to read lock if the count is zero
// Each element in the writeLocks list is a lock if the count is nonzero or a request to write lock if the count is zero
// Only the head element in the writeLocks list can have a nonzero count
//
// Lock records come from a free list. When it is empty a record has to be allocated, which calls malloc. This must not happen inside the
// task critical section, for the following reason.
// malloc and free are protected by a mutex of their own, see GetMallocMutex in the firmware. A mutex cannot be waited for while the scheduler
// is suspended, so GetMallocMutex does not take it in that state. An allocation made inside the critical section is therefore unprotected:
// 1. A low priority task, e.g. Main creating a heater while it executes config.g, is inside malloc and holds the malloc mutex.
// 2. A higher priority task, e.g. Heat, preempts it and acquires a read lock for which it needs a new lock record.
// 3. The free list is empty because more records are in use than ever before, so the record has to be allocated.
// 4. If that allocation is made inside the critical section, malloc runs without the mutex while the first task is halfway through changing
//    the heap, and the heap gets corrupted.
// NewLockRecord therefore leaves the critical section before it allocates and enters it again afterwards. With the scheduler running, malloc
// takes its mutex as usual: if nobody else is inside malloc it returns at once, otherwise this task waits on the mutex until the other task
// has left malloc. Taking a record from the free list, which is the normal case, is done without leaving the critical section.
// Because the critical section may have been left, the lock functions must always allow for that. When changing them, keep to these rules:
// - A record is added to and removed from the lists only by the task that owns it. Checks about the caller's own records made before leaving
//   the critical section therefore still hold after entering it again.
// - Everything else (the list heads, whether a writer holds or waits for the lock) may have changed and must be read again after entering.
// - Linking a record and deciding whether it is granted must happen in the same critical section, otherwise a wake-up can be missed.
// - The conditional functions test their condition again after allocating and delete the record if it no longer holds. Deleting only returns
//   the record to its free list, so that is safe inside the critical section.

#ifdef RTOS

// Structure used to record a task that has a lock or want to have one, and how many times it has acquired the lock (0 = still waiting)
struct LockRecord
{
	DECLARE_FREELIST_NEW_DELETE(LockRecord);

	LockRecord *_ecv_null next;
	TaskBase *_ecv_from null owner;
	uint32_t count;

	LockRecord(LockRecord *_ecv_null p_next, TaskBase *_ecv_from null p_owner) noexcept
		: next(p_next), owner(p_owner), count(0) { }
};

// Get a new lock record for a task. The caller must be in the task critical section.
// If the free list is empty then this leaves the critical section while it allocates memory, so the caller must read the lock lists again afterwards
static LockRecord *NewLockRecord(TaskBase *_ecv_from p_owner) noexcept
{
	void *_ecv_null p = FreelistManager::TryAllocate<LockRecord>();	// normal case: take a record from the free list, this never calls malloc
	if (p == nullptr)
	{
		// The free list is empty, so we need malloc. Let the scheduler run while we call it, so that malloc takes its mutex and waits
		// if another task is inside malloc, see the explanation at the start of this section
		RTOSIface::LeaveTaskCriticalSection();
		p = FreelistManager::Allocate<LockRecord>();
		RTOSIface::EnterTaskCriticalSection();						// other tasks may have changed the lock lists by now
	}
	return ::new (p) LockRecord(nullptr, p_owner);
}

void ReadWriteLock::LockForReading() noexcept
{
	TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();

	// If we own the write lock, ignore the read lock request
	{
		LockRecord *_ecv_null const wl = writeLocks;					// capture volatile variable
		if (wl != nullptr && wl->owner == me)
		{
			RTOSIface::LeaveTaskCriticalSection();
			return;
		}
	}

	// Check whether we already have a read lock, if we do then just increment the count
	for (LockRecord *_ecv_null rl = readLocks; rl != nullptr; rl = rl->next)
	{
		if (rl->owner == me)
		{
			++rl->count;
			RTOSIface::LeaveTaskCriticalSection();
			return;
		}
	}

	// We don't already own a read lock, so we need a new lock record.
	// Getting one may leave the critical section, but only this task can add or remove its own records, so the checks above still hold afterwards
	LockRecord *const lr = NewLockRecord(me);
	lr->next = readLocks;
	readLocks = lr;

	// If nobody owns or is trying to acquire write lock, we can read lock
	if (writeLocks == nullptr)
	{
		++lr->count;
		RTOSIface::LeaveTaskCriticalSection();
		return;
	}

	// If we get here we must wait until we are notified
	RTOSIface::LeaveTaskCriticalSection();
	do
	{
		TaskBase::TakeIndexed(NotifyIndices::ReadWriteLocker);
	} while (lr->count == 0);
}

bool ReadWriteLock::ConditionalLockForReading() noexcept
{
	TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();

	// If we own the write lock, ignore the read lock request
	{
		LockRecord *_ecv_null const wl = writeLocks;					// capture volatile variable
		if (wl != nullptr && wl->owner == me)
		{
			RTOSIface::LeaveTaskCriticalSection();
			return true;
		}
	}

	// Check whether we already have a read lock, if we do then just increment the count
	for (LockRecord *_ecv_null readOwner = readLocks; readOwner != nullptr; readOwner = readOwner->next)
	{
		if (readOwner->owner == me)
		{
			++readOwner->count;
			RTOSIface::LeaveTaskCriticalSection();
			return true;
		}
	}

	// We don't already own a read lock, so we need a new lock record

	// If nobody owns or is trying to acquire write lock, we can read lock
	if (writeLocks == nullptr)
	{
		LockRecord *const lr = NewLockRecord(me);
		if (writeLocks == nullptr)									// test again, getting the record may have left the critical section
		{
			lr->next = readLocks;
			readLocks = lr;
			lr->count++;
			RTOSIface::LeaveTaskCriticalSection();
			return true;
		}
		delete lr;													// another task asked for the write lock in the meantime
	}

	RTOSIface::LeaveTaskCriticalSection();
	return false;
}

void ReadWriteLock::ReleaseReader() noexcept
{
	TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();
	const LockRecord *_ecv_null const wl = writeLocks;			// capture volatile variable
	if (wl == nullptr || wl->owner != me)				// if we own the write lock, ignore the read-unlock
	{
		// Check whether we already have a read lock, if we do then just increment the count
		bool foundOwnRecord = false;
		LockRecord *_ecv_null prev = nullptr;
		bool hasRemainingReadLocks = false;
		for (LockRecord *_ecv_null rl = readLocks; rl != nullptr; )
		{
			if (rl->owner == me)
			{
				foundOwnRecord = true;
				if ((--rl->count) == 0)
				{
					if (prev == nullptr)
					{
						readLocks = rl->next;
					}
					else
					{
						prev->next = rl->next;
					}
					LockRecord *tbd = not_null(rl);
					rl = rl->next;
					delete tbd;
					if (hasRemainingReadLocks)
					{
						break;
					}
				}
				else
				{
					hasRemainingReadLocks = true;
					break;
				}
			}
			else
			{
				if (rl->count != 0)
				{
					hasRemainingReadLocks = true;
				}
				prev = rl;
				rl = rl->next;
			}
		}

		RTOS_ASSERT(foundOwnRecord);
		if (!hasRemainingReadLocks)
		{
			LockRecord *_ecv_null const wr = writeLocks;		// capture volatile variable
			if (wr != nullptr && wr->count == 0)
			{
				++wr->count;
				wr->owner->Give(NotifyIndices::ReadWriteLocker);
			}
		}
	}
	RTOSIface::LeaveTaskCriticalSection();
}

void ReadWriteLock::LockForWriting() noexcept
{
	TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();

	LockRecord *_ecv_null wr = writeLocks;
	if (wr != nullptr && wr->owner == me)
	{
		RTOS_ASSERT(wr->count != 0);
		++wr->count;
		RTOSIface::LeaveTaskCriticalSection();
		return;
	}

	// We need a new lock record
	LockRecord *newLock = NewLockRecord(me);
	wr = writeLocks;												// read it again, getting the record may have left the critical section
	if (wr == nullptr)
	{
		// Nobody else is waiting for the write lock, so make the write lock list point to our lock record
		writeLocks = newLock;

		// If there are no read locks, we can grab it. Any that exist must have a nonzero count.
		if (readLocks == nullptr)
		{
			++newLock->count;
			RTOSIface::LeaveTaskCriticalSection();
			return;
		}
	}
	else
	{
		// Add ourselves to the queue of tasks waiting for a write lock
		while (wr->next != nullptr)
		{
			wr = wr->next;
		}
		wr->next = newLock;
	}

	RTOSIface::LeaveTaskCriticalSection();

	// Wait until we are given the lock
	do
	{
		TaskBase::TakeIndexed(NotifyIndices::ReadWriteLocker);
	}
	while (newLock->count == 0);
}

bool ReadWriteLock::ConditionalLockForWriting() noexcept
{
	TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();

	if (writeLocks == nullptr && readLocks == nullptr)
	{
		LockRecord *newLock = NewLockRecord(me);
		if (writeLocks == nullptr && readLocks == nullptr)			// test again, getting the record may have left the critical section
		{
			// Nobody else is waiting for the write lock and there are no read locks
			writeLocks = newLock;
			newLock->count++;
			RTOSIface::LeaveTaskCriticalSection();
			return true;
		}
		delete newLock;												// another task took a lock in the meantime
	}

	RTOSIface::LeaveTaskCriticalSection();
	return false;
}

void ReadWriteLock::ReleaseWriter() noexcept
{
	TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();

	LockRecord *_ecv_null wl = writeLocks;
	RTOS_ASSERT(wl != nullptr && wl->owner == me && wl->count != 0);
	if (--wl->count == 0)
	{
		LockRecord *_ecv_null wl2 = wl->next;
		writeLocks = wl2;
		delete wl;

		if (wl2 != nullptr)
		{
			// Another task is waiting for a write lock so pass the lock on to it
			++wl2->count;
			wl2->owner->Give(NotifyIndices::ReadWriteLocker);
		}
		else
		{
			// Wake up any tasks waiting for read locks
			for (LockRecord *_ecv_null rl = readLocks; rl != nullptr; rl = rl->next)
			{
				if (rl->count == 0)			// this should always be true
				{
					++rl->count;
					rl->owner->Give(NotifyIndices::ReadWriteLocker);
				}
			}
		}
	}
	RTOSIface::LeaveTaskCriticalSection();
}

void ReadWriteLock::DowngradeWriter() noexcept
{
	TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();
	LockRecord *_ecv_null const wl = writeLocks;
	RTOS_ASSERT(wl != nullptr && wl->owner == me && wl->count == 1);
	writeLocks = wl->next;
	wl->next = readLocks;
	readLocks = wl;

	// Wake up any other tasks waiting for read locks
	for (LockRecord *_ecv_null rl = wl->next; rl != nullptr; rl = rl->next)
	{
		if (rl->count == 0)			// this should always be true
		{
			++rl->count;
			rl->owner->Give(NotifyIndices::ReadWriteLocker);
		}
	}
	RTOSIface::LeaveTaskCriticalSection();
}

// Return true if there is an active write lock. Must only be called with interrupts disabled or scheduling disabled. Safe to call from an ISR.
bool ReadWriteLock::IsWriteLocked() const noexcept
{
	return writeLocks != nullptr && writeLocks->count != 0;
}

void ReadWriteLock::CheckHasWriteLock() noexcept
{
	const TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();
	LockRecord *_ecv_null wl = writeLocks;
	RTOS_ASSERT(wl != nullptr && wl->owner == me && wl->count != 0);
	RTOSIface::LeaveTaskCriticalSection();
}

void ReadWriteLock::CheckHasReadLock() noexcept
{
	const TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();
	LockRecord *_ecv_null rl = readLocks;
	while (rl != nullptr && rl->owner != me)
	{
		rl = rl->next;
	}
	RTOS_ASSERT(rl != nullptr && rl->count != 0);
	RTOSIface::LeaveTaskCriticalSection();
}

void ReadWriteLock::CheckHasReadOrWriteLock() noexcept
{
	const TaskBase *_ecv_from const me = not_null(TaskBase::GetCallerTaskHandle());
	RTOSIface::EnterTaskCriticalSection();
	LockRecord *_ecv_null wl = writeLocks;
	if (wl == nullptr || wl->owner != me || wl->count == 0)
	{
		LockRecord *_ecv_null rl = readLocks;
		while (rl != nullptr && rl->owner != me)
		{
			rl = rl->next;
		}
		RTOS_ASSERT(rl != nullptr && rl->count != 0);
	}
	RTOSIface::LeaveTaskCriticalSection();
}

#endif
// End
