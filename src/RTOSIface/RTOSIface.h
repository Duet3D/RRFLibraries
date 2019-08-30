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

#ifdef RTOS
# include "FreeRTOS.h"
# include "task.h"
# include "semphr.h"
#else

/** \brief  Enable IRQ Interrupts

  This function enables IRQ interrupts by clearing the I-bit in the CPSR.
  Can only be executed in Privileged modes.
 */
__attribute__( ( always_inline ) ) static inline void __enable_irq()
{
  __asm volatile ("cpsie i" : : : "memory");
}


/** \brief  Disable IRQ Interrupts

  This function disables IRQ interrupts by setting the I-bit in the CPSR.
  Can only be executed in Privileged modes.
 */
__attribute__( ( always_inline ) ) static inline void __disable_irq()
{
  __asm volatile ("cpsid i" : : : "memory");
}

#endif

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

	// Wake up this task from an ISR
	void GiveFromISR()
	{
		BaseType_t higherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(handle, &higherPriorityTaskWoken);
		portYIELD_FROM_ISR(higherPriorityTaskWoken);
	}

	// Wake up a task identified by its handle from an ISR
	static inline void GiveFromISR(TaskHandle h)
	{
		BaseType_t higherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(h, &higherPriorityTaskWoken);
		portYIELD_FROM_ISR(higherPriorityTaskWoken);
	}

	// Wake up this task but not from an ISR
	void Give()
	{
		xTaskNotifyGive(handle);
	}

	static void Give(TaskHandle handle)
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
		__disable_irq();
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
			__enable_irq();
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
};

class TaskCriticalSectionLocker
{
public:
	TaskCriticalSectionLocker() { RTOSIface::EnterTaskCriticalSection(); }
	~TaskCriticalSectionLocker() { RTOSIface::LeaveTaskCriticalSection(); }
};

#endif /* SRC_RTOSIFACE_H_ */
