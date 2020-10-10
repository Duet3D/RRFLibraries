/*
 * RTOS.cpp
 *
 *  Created on: 30 Mar 2018
 *      Author: David
 */

#include "RTOSIface.h"

#ifdef RTOS

# include "FreeRTOS.h"
# include "task.h"
# include "semphr.h"
# include <atomic>

static_assert(Mutex::TimeoutUnlimited == portMAX_DELAY, "Bad value for TimeoutUnlimited");

void Mutex::Create(const char *pName) noexcept
{
	if (handle == nullptr)
	{
		handle = xSemaphoreCreateRecursiveMutexStatic(&storage);
		name = pName;
		next = mutexList;
		mutexList = this;
	}
}

// Take ownership of a mutex returning true if successful, false if timed out
bool Mutex::Take(uint32_t timeout) const noexcept
{
	return xSemaphoreTakeRecursive(handle, timeout) == pdTRUE;
}

// Release a mutex returning true if successful.
// Note that the return value does not indicate whether the mutex is still owned, because it may have been taken more than once.
bool Mutex::Release() const noexcept
{
	return xSemaphoreGiveRecursive(handle) == pdTRUE;
}

TaskHandle Mutex::GetHolder() const noexcept
{
	return static_cast<TaskHandle>(xSemaphoreGetMutexHolder(handle));
}

TaskBase *TaskBase::taskList = nullptr;
TaskBase::TaskId TaskBase::numTasks = 0;

Mutex *Mutex::mutexList = nullptr;

#else

void Mutex::Create(const char *pName) noexcept
{
}

bool Mutex::Take(uint32_t timeout) const noexcept
{
	return true;
}

bool Mutex::Release() const noexcept
{
	return true;
}

TaskHandle Mutex::GetHolder() const noexcept
{
	return nullptr;
}

#endif

MutexLocker::MutexLocker(const Mutex *m, uint32_t timeout) noexcept
{
	handle = m;
	acquired =
#ifdef RTOS
				(m == nullptr) || m->Take(timeout);
#else
				true;
#endif
}

MutexLocker::MutexLocker(const Mutex& m, uint32_t timeout) noexcept
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
			handle->Release();
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
		acquired = (handle == nullptr) || handle->Take(timeout);
	}
#else
	acquired = true;
#endif
	return acquired;
}

MutexLocker::~MutexLocker() noexcept
{
	Release();
}

#ifdef RTOS

BinarySemaphore::BinarySemaphore() noexcept
{
	 handle = xSemaphoreCreateBinaryStatic(&storage);
}

bool BinarySemaphore::Take(uint32_t timeout) const noexcept
{
	return xSemaphoreTake(handle, timeout);
}

bool BinarySemaphore::Give() const noexcept
{
	return xSemaphoreGive(handle);
}

// Link the task into the thread list and allocate a short task ID to it. Task IDs start at 1.
void TaskBase::AddToList() noexcept
{
	TaskCriticalSectionLocker lock;

	++numTasks;
	taskId = numTasks;
	handle = reinterpret_cast<TaskHandle>(&storage);
	next = taskList;
	taskList = this;
}

// Get the short-form task ID
/*static*/ TaskBase::TaskId TaskBase::GetCallerTaskId() noexcept
{
	TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();

	// We need to get the task ID given the task handle.
	// We could cheat and rely on the fact the the task ID should be 4 bytes before the storage that the task handle points to.
	// But we'll do it properly and search the task list instead.
	for (TaskBase** tpp = &taskList; *tpp != nullptr; tpp = &(*tpp)->next)
	{
		if ((*tpp)->handle == currentTaskHandle)
		{
			return (*tpp)->taskId;
		}
	}
	return 0;				// won't happen unless the current task hasn't been linked into the task list
}

// Terminate a task and remove it from the thread list
void TaskBase::TerminateAndUnlink() noexcept
{
	if (handle != nullptr)
	{
		vTaskDelete(handle);
		handle = nullptr;

		// Unlink the task from the thread list
		TaskCriticalSectionLocker lock;
		for (TaskBase** tpp = &taskList; *tpp != nullptr; tpp = &(*tpp)->next)
		{
			if (*tpp == this)
			{
				*tpp = (*tpp)->next;
				break;
			}
		}
	}
}

#endif

namespace RTOSIface
{

#ifdef RTOS

	TaskHandle GetCurrentTask() noexcept
	{
		return static_cast<TaskHandle>(xTaskGetCurrentTaskHandle());
	}

#else

	TaskHandle GetCurrentTask() noexcept
	{
		return nullptr;
	}

#endif

}

void ReadWriteLock::LockForReading() noexcept
{
#ifdef RTOS
	if (writeLockOwner != TaskBase::GetCallerTaskHandle())
	{
		for (;;)
		{
# if RRFLIBS_SAMC21
			DisableInterrupts();
			const uint8_t nr = numReaders;
			if ((nr & 0x80) == 0)
			{
				numReaders = nr + 1;
				EnableInterrupts();
				break;
			}
			EnableInterrupts();
			vTaskDelay(1);
# else
			uint8_t nr = numReaders;
			if (nr & 0x80)
			{
				vTaskDelay(1);					// delay while writing is pending or active
			}
			else if (numReaders.compare_exchange_strong(nr, nr + 1))
			{
				break;
			}
# endif
		}
	}
#endif
}

void ReadWriteLock::ReleaseReader() noexcept
{
#ifdef RTOS
	if (writeLockOwner != TaskBase::GetCallerTaskHandle())
	{
# if RRFLIBS_SAMC21
		DisableInterrupts();
		--numReaders;
		EnableInterrupts();
# else
		--numReaders;
# endif
	}
#endif
}

void ReadWriteLock::LockForWriting() noexcept
{
#ifdef RTOS
	// First wait for other writers to finish, then grab the write lock
	for (;;)
	{
# if RRFLIBS_SAMC21
		DisableInterrupts();
		const uint8_t nr = numReaders;
		if ((nr & 0x80) == 0)
		{
			numReaders = nr | 0x80;
			EnableInterrupts();
			break;
		}
		EnableInterrupts();
		vTaskDelay(1);
# else
		uint8_t nr = numReaders;
		if (nr & 0x80)
		{
			vTaskDelay(1);					// delay while writing is pending or active
		}
		else if (numReaders.compare_exchange_strong(nr, nr | 0x80))
		{
			break;
		}
# endif
	}

	// Now wait for readers to finish
	while (numReaders != 0x80)
	{
		vTaskDelay(1);
	}

	writeLockOwner = TaskBase::GetCallerTaskHandle();
#endif
}

void ReadWriteLock::ReleaseWriter() noexcept
{
#ifdef RTOS
	if (writeLockOwner == TaskBase::GetCallerTaskHandle())
	{
		writeLockOwner = nullptr;
		numReaders = 0;
	}
	else if ((numReaders & 0x7F) != 0)
	{
		// We must have downgraded to a read lock
		--numReaders;
	}
#endif
}

void ReadWriteLock::DowngradeWriter() noexcept
{
#ifdef RTOS
	if (writeLockOwner == TaskBase::GetCallerTaskHandle())
	{
		numReaders = 1;
		writeLockOwner = nullptr;
	}
#endif
}

// End
