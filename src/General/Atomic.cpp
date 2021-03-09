/*
 * Atomic.cpp
 *
 *  Created on: 9 Mar 2021
 *      Author: David
 */

// Atomic functions for ARM Cortex M0/M0+ processors missing from the standard library
// There are many other such functions missing. They can be added if/when the linker complains about not finding them.
// See section 6.55 of the gcc manual for more information about them.

#if (defined(__SAMC21G18A__) && __SAMC21G18A__)

#include <Core.h>

extern "C" uint8_t __atomic_sub_fetch_1(volatile void *ptr, uint8_t val, int memorder) noexcept
{
	const irqflags_t flags = cpu_irq_save();
	uint8_t ret = *(volatile uint8_t*)ptr;
	ret -= val;
	*(volatile uint8_t*)ptr = ret;
	cpu_irq_restore(flags);
	return ret;
}

extern "C" uint8_t __atomic_fetch_sub_1(volatile void *ptr, uint8_t val, int memorder) noexcept
{
	const irqflags_t flags = cpu_irq_save();
	const uint8_t ret = *(volatile uint8_t*)ptr;
	*(volatile uint8_t*)ptr = ret - val;
	cpu_irq_restore(flags);
	return ret;
}

extern "C" bool __atomic_compare_exchange_1(volatile void *ptr, void *expected, uint8_t desired, bool weak, int success_memorder, int failure_memorder) noexcept
{
	bool ret;
	const irqflags_t flags = cpu_irq_save();
	const uint8_t actual = *(volatile uint8_t*)ptr;
	if (*(uint8_t*)expected == actual)
	{
		*(volatile uint8_t*)ptr = desired;
		ret = true;
	}
	else
	{
		*(uint8_t*)expected = actual;
		ret = false;
	}
	cpu_irq_restore(flags);
	return ret;
}

extern "C" unsigned int __atomic_fetch_or_4(volatile void *ptr, unsigned int val, int memorder) noexcept
{
	const irqflags_t flags = cpu_irq_save();
	const unsigned int ret = *(volatile unsigned int*)ptr;
	*(volatile unsigned int*)ptr = ret | val;
	cpu_irq_restore(flags);
	return ret;
}

extern "C" unsigned int __atomic_fetch_and_4(volatile void *ptr, unsigned int val, int memorder) noexcept
{
	const irqflags_t flags = cpu_irq_save();
	const unsigned int ret = *(volatile unsigned int*)ptr;
	*(volatile unsigned int*)ptr = ret & val;
	cpu_irq_restore(flags);
	return ret;
}

#endif

// End
