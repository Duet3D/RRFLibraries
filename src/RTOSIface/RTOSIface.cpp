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

static_assert(Mutex::TimeoutUnlimited == portMAX_DELAY, "Bad value for TimeoutUnlimited");

void Mutex::Create(const char *pName)
{
	if (handle == nullptr)
	{
		handle = xSemaphoreCreateRecursiveMutexStatic(&storage);
		name = pName;
		next = mutexList;
		mutexList = this;
	}
}

// Take ownership of a mutex returning true if successful
bool Mutex::Take(uint32_t timeout) const
{
	return xSemaphoreTakeRecursive(handle, timeout) == pdTRUE;
}

// Release a mutex returning true if successful.
// Note that the return value does not indicate whether the mutex is still owned, because it may have been taken more than once.
bool Mutex::Release() const
{
	return xSemaphoreGiveRecursive(handle) == pdTRUE;
}

TaskHandle Mutex::GetHolder() const
{
	return static_cast<TaskHandle>(xSemaphoreGetMutexHolder(handle));
}

TaskBase *TaskBase::taskList = nullptr;
TaskBase::TaskId TaskBase::numTasks = 0;

Mutex *Mutex::mutexList = nullptr;

#else

void Mutex::Create(const char *pName)
{
}

bool Mutex::Take(uint32_t timeout) const
{
	return true;
}

bool Mutex::Release() const
{
	return true;
}

TaskHandle Mutex::GetHolder() const
{
	return nullptr;
}

#endif

MutexLocker::MutexLocker(const Mutex *m, uint32_t timeout)
{
	handle = m;
	acquired =
#ifdef RTOS
				m == nullptr || m->Take(timeout);
#else
				true;
#endif
}

MutexLocker::MutexLocker(const Mutex& m, uint32_t timeout)
{
	handle = &m;
	acquired =
#ifdef RTOS
				m.Take(timeout);
#else
				true;
#endif
}

void MutexLocker::Release()
{
#ifdef RTOS
	if (acquired && handle != nullptr)
	{
		handle->Release();
		acquired = (handle->GetHolder() == TaskBase::GetCallerTaskHandle());
	}
#endif
}

MutexLocker::~MutexLocker()
{
	Release();
#ifdef RTOS
	if (acquired && handle != nullptr)
	{
		handle->Release();
	}
#endif
}

#ifdef RTOS

// Link the task into the thread list and allocate a short task ID to it. Task IDs start at 1.
void TaskBase::AddToList()
{
	TaskCriticalSectionLocker lock;

	++numTasks;
	taskId = numTasks;
	handle = &storage;
	next = taskList;
	taskList = this;
}

// Terminate a task and remove it from the thread list
void TaskBase::TerminateAndUnlink()
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

// Get the short-form task ID
/*static*/ TaskBase::TaskId TaskBase::GetCallerTaskId()
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
	return 0;				// won't happen unless the current task hasn't been linked nito the task list
}

#endif

namespace RTOSIface
{

#ifdef RTOS

	TaskHandle GetCurrentTask()
	{
		return static_cast<TaskHandle>(xTaskGetCurrentTaskHandle());
	}

#else

	TaskHandle GetCurrentTask()
	{
		return nullptr;
	}

#endif

}

// End
