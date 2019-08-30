/*
 * Portability.h
 *
 *  Created on: 30 Aug 2019
 *      Author: David
 */

#ifndef SRC_GENERAL_PORTABILITY_H_
#define SRC_GENERAL_PORTABILITY_H_

#include <cstdint>

// Functions to allow for processor differences, e.g. endianness and alignment requirements

// Load a uint32 from unaligned memory in little endian format
static inline uint32_t LoadLE32(const void *p)
{
	return *reinterpret_cast<const uint32_t*>(p);			// the processors we currently support are little endian and support unaligned accesses
}

// Load a uint16 from unaligned memory in little endian format
static inline uint16_t LoadLE16(const void *p)
{
	return *reinterpret_cast<const uint16_t*>(p);			// the processors we currently support are little endian and support unaligned accesses
}

// Load a float from unaligned memory in little endian format
static inline float LoadLEFloat(const void *p)
{
	return *reinterpret_cast<const float*>(p);				// the processors we currently support are little endian and support unaligned accesses
}

// Store a uint32 into unaligned memory in little endian format
static inline void StoreLE32(void *p, uint32_t val)
{
	*reinterpret_cast<uint32_t*>(p) = val;					// the processors we currently support are little endian and support unaligned accesses
}

// Store a uint16 into unaligned memory in little endian format
static inline void StoreLE16(void *p, uint16_t val)
{
	*reinterpret_cast<uint16_t*>(p) = val;					// the processors we currently support are little endian and support unaligned accesses
}

// Store a uint16 into unaligned memory in little endian format
static inline void StoreLEFloat(void *p, float val)
{
	*reinterpret_cast<float*>(p) = val;						// the processors we currently support are little endian and support unaligned accesses
}

// Load a uint32 from unaligned memory in big endian format
static inline uint32_t LoadBE32(const void *p)
{
	const uint8_t* const bp = (const uint8_t*)p;
	return ((((((uint32_t)*bp << 8) | (uint32_t)*(bp + 1)) << 8) | (uint32_t)*(bp + 2)) << 8) | (uint32_t)*(bp + 3);
}

// Load a uint16 from unaligned memory in big endian format
static inline uint16_t LoadBE16(const void *p)
{
	const uint8_t* const bp = (const uint8_t*)p;
	return ((uint16_t)*bp << 8) | (uint16_t)*(bp + 1);
}

// Store a uint32 into unaligned memory in big endian format
static inline void StoreBE32(void *p, uint32_t val)
{
	uint8_t* bp = (uint8_t*)p;
	*bp++ = (uint8_t)(val >> 24);
	*bp++ = (uint8_t)(val >> 16);
	*bp++ = (uint8_t)(val >> 8);
	*bp = (uint8_t)val;
}

// Store a uint16 into unaligned memory in big endian format
static inline void StoreBE16(void *p, uint16_t val)
{
	uint8_t* bp = (uint8_t*)p;
	*bp++ = (uint8_t)(val >> 8);
	*bp = (uint8_t)val;
}

#endif /* SRC_GENERAL_PORTABILITY_H_ */
