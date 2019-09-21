/*
 * RTOS.h
 *
 *  Created on: 30 Mar 2018
 *      Author: David
 */

#ifndef SRC_RTOSIFACE_H_
#define SRC_RTOSIFACE_H_

#include <cstdint>

// Type declarations to hide the type-unsafe definitions in the FreeRTOS headers

class Task_undefined;					// this class is never defined
typedef Task_undefined *TaskHandle;

#include <utility>

#ifdef RTOS
# include "FreeRTOS.h"
# include "task.h"
# include "semphr.h"
# include <atomic>
#endif

#define RRFLIBS_SAMC21	(defined(__SAMC21G18A__) && __SAMC21G18A__)

/** \brief  Enable IRQ Interrupts

  This function enables IRQ interrupts by clearing the I-bit in the CPSR.
  Can only be executed in Privileged modes.
 */
__attribute__( ( always_inline ) ) static inline void EnableInterrupts()
{
  __asm volatile ("cpsie i" : : : "memory");
}


/** \brief  Disable IRQ Interrupts

  This function disables IRQ interrupts by setting the I-bit in the CPSR.
  Can only be executed in Privileged modes.
 */
__attribute__( ( always_inline ) ) static inline void DisableInterrupts()
{
  __asm volatile ("cpsid i" : : : "memory");
}

class Mutex
{
public:
	Mutex() : handle(nullptr)
#ifdef RTOS
		, next(nullptr), name(nullptr)
#endif
	{ }

	void Create(const char *pName);
	bool Take(uint32_t timeout = TimeoutUnlimited) const;
	bool Release() const;
	TaskHandle GetHolder() const;

#ifdef RTOS
	const Mutex *GetNext() const { return next; }
	const char *GetName() const { return name; }
#endif

	Mutex(const Mutex&) = delete;
	Mutex& operator=(const Mutex&) = delete;

	static constexpr uint32_t TimeoutUnlimited = 0xFFFFFFFF;

#ifdef RTOS
	static const Mutex *GetMutexList() { return mutexList; }
#endif

private:

#ifdef RTOS
	SemaphoreHandle_t handle;
	Mutex *next;
	const char *name;
	StaticSemaphore_t storage;

	static Mutex *mutexList;
#else
	void *handle;
#endif

};

class BinarySemaphore
{
public:
	BinarySemaphore();

	bool Take(uint32_t timeout = TimeoutUnlimited) const;
	bool Give() const;

	static constexpr uint32_t TimeoutUnlimited = 0xFFFFFFFF;

private:

#ifdef RTOS
	SemaphoreHandle_t handle;
	StaticSemaphore_t storage;
#endif
};

#ifdef RTOS

class TaskBase
{
public:
	// Type of a short-form task ID. Task IDs start at 1 and each task takes the next available number.
	// If tasks are never deleted except at shutdown then we can guarantee that task IDs will be small number, because it won't exceed the number of tasks.
	// This is used by the CAN subsystem, so that we can use 8-bit task IDs to identify a sending task, instead of needing to use 32-bits.
	typedef uint32_t TaskId;

	TaskBase() : handle(nullptr), next(nullptr) { }
	~TaskBase() { TerminateAndUnlink(); }

	// Get the short-form task ID. This is a small number, used to send a task ID in 1 byte or less i a CAN packet. It is guaranteed not to be zero.
	TaskId GetTaskId() const { return taskId; }

	// This function is called directly for tasks that are created by FreeRTOS, so it must be public
	// Link the task into the thread list and allocate a short task ID to it
	void AddToList();

	void TerminateAndUnlink();

	TaskHandle GetHandle() const { return static_cast<TaskHandle>(handle); }
	void Suspend() const { vTaskSuspend(handle); }
	void Resume() const { vTaskResume(handle); }
	const TaskBase *GetNext() const { return next; }

	// Wake up a task identified by its handle from an ISR
	static inline void GiveFromISR(TaskHandle h)
	{
		if (h != nullptr)			// check that the task has been created
		{
			BaseType_t higherPriorityTaskWoken = pdFALSE;
			vTaskNotifyGiveFromISR(h, &higherPriorityTaskWoken);
			portYIELD_FROM_ISR(higherPriorityTaskWoken);
		}
	}

	// Wake up this task from an ISR
	void GiveFromISR()
	{
		if (handle != nullptr)			// check that the task has been created
		{
			BaseType_t higherPriorityTaskWoken = pdFALSE;
			vTaskNotifyGiveFromISR(handle, &higherPriorityTaskWoken);
			portYIELD_FROM_ISR(higherPriorityTaskWoken);
		}
	}

	// Wake up this task but not from an ISR
	void Give()
	{
		xTaskNotifyGive(handle);
	}

	// Wait until we have been woken up
	static uint32_t Take(uint32_t timeout = TimeoutUnlimited)
	{
		return ulTaskNotifyTake(pdTRUE, timeout);
	}

	static TaskHandle GetCallerTaskHandle() { return (TaskHandle)xTaskGetCurrentTaskHandle(); }

	static TaskId GetCallerTaskId();

	TaskBase(const TaskBase&) = delete;				// it's not safe to copy these
	TaskBase& operator=(const TaskBase&) = delete;	// it's not safe to assign these
	// Ideally we would declare the destructor as deleted too, because it's unsafe to delete these because they are linked together via the 'next' field.
	// But that prevents us from declaring static instances of tasks.
	// Possible solutions:
	// 1. Just be careful that after we allocate a task using 'new', we never delete it.
	// 2. Write a destructor that removes the task from the linked list.
	// 3. Don't allocate tasks statically, allocate them all using 'new'.
	//~TaskBase() = delete;							// it's not safe to delete these because they are linked together via the 'next' field

	static const TaskBase *GetTaskList() { return taskList; }

	static constexpr uint32_t TimeoutUnlimited = 0xFFFFFFFF;

protected:
	TaskHandle_t handle;
	TaskBase *next;
	TaskId taskId;
	StaticTask_t storage;

	static TaskBase *taskList;
	static TaskId numTasks;
};

template<unsigned int StackWords> class Task : public TaskBase
{
public:
	// The Create function assumes that only the main task creates other tasks, so we don't need a mutex to protect the task list
	void Create(TaskFunction_t pxTaskCode, const char * pcName, void *pvParameters, unsigned int uxPriority)
	{
		handle = xTaskCreateStatic(pxTaskCode, pcName, StackWords, pvParameters, uxPriority, stack, &storage);
		AddToList();
	}

	// These functions should be used only to tell FreeRTOS where the corresponding data is
	StaticTask_t *GetTaskMemory() { return &storage; }
	uint32_t *GetStackBase() { return stack; }
	uint32_t GetStackSize() const { return StackWords; }

private:
	uint32_t stack[StackWords];
};

#endif

// Class to lock a mutex and automatically release it when it goes out of scope
// If we pass a null mutex handle to the Locker constructor, it means there is nothing to lock and we pretend the lock has been acquired.
class MutexLocker
{
public:
	MutexLocker(const Mutex *pm, uint32_t timeout = Mutex::TimeoutUnlimited);	// acquire lock
	MutexLocker(const Mutex& pm, uint32_t timeout = Mutex::TimeoutUnlimited);	// acquire lock

	void Release();																// release the lock early (also gets released by destructor)
	bool ReAcquire(uint32_t timeout = Mutex::TimeoutUnlimited);					// acquire it again, if it isn't already owned (non-counting)
	~MutexLocker();
	operator bool() const { return acquired; }

	MutexLocker(const MutexLocker&) = delete;
	MutexLocker& operator=(const MutexLocker&) = delete;
	MutexLocker(MutexLocker&& other) : handle(other.handle), acquired(other.acquired) { other.handle = nullptr; other.acquired = false; }

private:
	const Mutex *handle;
	bool acquired;
};

// Interface to RTOS or RTOS substitute
namespace RTOSIface
{
	TaskHandle GetCurrentTask();

#ifndef RTOS
	static volatile unsigned int interruptCriticalSectionNesting = 0;
#endif

	// Enter a critical section, where modification to variables by interrupts (and perhaps also other tasks) must be avoided
	inline void EnterInterruptCriticalSection()
	{
#ifdef RTOS
		taskENTER_CRITICAL();
#else
		DisableInterrupts();
		++interruptCriticalSectionNesting;
#endif
	}

	// Leave an interrupt-critical section
	inline void LeaveInterruptCriticalSection()
	{
#ifdef RTOS
		taskEXIT_CRITICAL();
#else
		--interruptCriticalSectionNesting;
		if (interruptCriticalSectionNesting == 0)
		{
			EnableInterrupts();
		}
#endif
	}

	// Enter a task-critical region. Used to protect concurrent access to variable from different tasks, where the variable are not used/modified by interrupts.
	// This can be called even if the caller is already in a TaskCriticalSection, because FreeRTOS keeps track of the nesting count.
	inline void EnterTaskCriticalSection()
	{
#ifdef RTOS
		vTaskSuspendAll();
#else
		// nothing to do here because there is no task preemption
#endif
	}

	// Exit a task-critical region, returning true if a task switch occurred
	inline bool LeaveTaskCriticalSection()
	{
#ifdef RTOS
		return xTaskResumeAll() == pdTRUE;
#else
		// nothing to do here because there is no task preemption
		return false;
#endif
	}

	inline void Yield()
	{
#ifdef RTOS
		taskYIELD();
#endif
	}
}

class InterruptCriticalSectionLocker
{
public:
	InterruptCriticalSectionLocker() { RTOSIface::EnterInterruptCriticalSection(); }
	~InterruptCriticalSectionLocker() { (void)RTOSIface::LeaveInterruptCriticalSection(); }

	InterruptCriticalSectionLocker(const InterruptCriticalSectionLocker&) = delete;
};

class TaskCriticalSectionLocker
{
public:
	TaskCriticalSectionLocker() { RTOSIface::EnterTaskCriticalSection(); }
	~TaskCriticalSectionLocker() { RTOSIface::LeaveTaskCriticalSection(); }

	TaskCriticalSectionLocker(const TaskCriticalSectionLocker&) = delete;
};

// Class to represent a lock that allows multiple readers but only one writer
// This is designed to be efficient when writing is rare
class ReadWriteLock
{
public:
	ReadWriteLock()
#ifdef RTOS
		: numReaders(0)
#endif
	{ }

	void LockForReading();
	void ReleaseReader();
	void LockForWriting();
	void ReleaseWriter();

private:

#ifdef RTOS
# if RRFLIBS_SAMC21
	volatile uint8_t numReaders;			// SAMC21 doesn't support atomic operations, neither does the library
# else
	std::atomic_uint8_t numReaders;			// MSB is set if a task is writing or write pending, lower bits are the number of readers
	static_assert(std::atomic_uint8_t::is_always_lock_free);
# endif
#endif
};

class ReadLocker
{
public:
	ReadLocker(ReadWriteLock& p_lock) : lock(&p_lock) { lock->LockForReading(); }
	~ReadLocker() { if (lock != nullptr) { lock->ReleaseReader(); } }

	ReadLocker(const ReadLocker&) = delete;
	ReadLocker(ReadLocker&& other) : lock(other.lock) { other.lock = nullptr; }

private:
	ReadWriteLock* lock;
};

class WriteLocker
{
public:
	WriteLocker(ReadWriteLock& p_lock) : lock(&p_lock) { lock->LockForWriting(); }
	~WriteLocker() { if (lock != nullptr) { lock->ReleaseWriter(); } }

	WriteLocker(const WriteLocker&) = delete;
	WriteLocker(WriteLocker&& other) : lock(other.lock) { other.lock = nullptr; }

private:
	ReadWriteLock* lock;
};

template<class T> class ReadLockedPointer
{
public:
	ReadLockedPointer(ReadLocker& p_locker, T* p_ptr) : locker(std::move(p_locker)), ptr(p_ptr) { }
	ReadLockedPointer(const ReadLockedPointer&) = delete;
	ReadLockedPointer(ReadLockedPointer&& other) : locker(other.locker), ptr(other.ptr) { other.ptr = nullptr; }

	bool IsNull() const { return ptr == nullptr; }
	bool IsNotNull() const { return ptr != nullptr; }
	T* operator->() const { return ptr; }

private:
	ReadLocker locker;
	T* ptr;
};

#endif /* SRC_RTOSIFACE_H_ */
