/*
 * gcc_builtins.h
 *
 *  Created on: 31 Oct 2021
 *      Author: David
 */

#ifndef SRC_GENERAL_GCC_BUILTINS_H_
#define SRC_GENERAL_GCC_BUILTINS_H_

// GCC builtin functions that we use. These are known to gcc but not to ecv.

#ifdef __ECV__

int __builtin_clz (unsigned int x) noexcept;
int __builtin_clzl (unsigned long) noexcept;
int __builtin_clzll (unsigned long long) noexcept;
int __builtin_ctz (unsigned int x) noexcept;
int __builtin_ctzl (unsigned long) noexcept;
int __builtin_ctzll (unsigned long long) noexcept;

#endif

#endif /* SRC_GENERAL_GCC_BUILTINS_H_ */
