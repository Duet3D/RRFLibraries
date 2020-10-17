/*
 * SafeVsnprintf.h
 *
 *  Created on: 8 Apr 2018
 *      Author: David
 */

#ifndef SRC_LIBRARIES_GENERAL_SAFEVSNPRINTF_H_
#define SRC_LIBRARIES_GENERAL_SAFEVSNPRINTF_H_

#include <cstdarg>
#include <cstddef>

# ifdef __cplusplus

# include <functional>

// Like printf but each character to print is sent through a function
int vuprintf(std::function<bool(char) /*noexcept*/ > putc, const char *format, va_list args) noexcept;
int uprintf(std::function<bool(char) /*noexcept*/ > putc, const char *format, ...) noexcept __attribute__ ((format (printf, 2, 3)));

# endif

int SafeVsnprintf(char *buffer, size_t maxLen, const char *format, va_list args) noexcept;
int SafeSnprintf(char* buffer, size_t maxLen, const char* format, ...) noexcept __attribute__ ((format (printf, 3, 4)));

// We must define these after including <functional> because that file refers to vsnprintf
#define vsnprintf(b, m, f, a) static_assert(false, "Do not use vsnprintf, use SafeVsnprintf instead")
#define snprintf(b, m, f, ...) static_assert(false, "Do not use snprintf, use SafeSnprintf instead")

#endif /* SRC_LIBRARIES_GENERAL_SAFEVSNPRINTF_H_ */
