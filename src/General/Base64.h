/*
 * Base64.h
 *
 *  Created on: 3 Mar 2021
 *      Author: manuel
 *
 *  Implementation "adition" as found at https://github.com/gaspardpetit/base64/
 */

#ifndef SRC_GENERAL_BASE64_H_
#define SRC_GENERAL_BASE64_H_

#if defined(WITH_BASE64_ENCODING) || defined(WITH_BASE64_DECODING)
#include <cstddef>	// for size_t
#include <cstdint>	// for uint8_t
#endif

namespace base64
{
#ifdef WITH_BASE64_ENCODING
	void EncodeChunk(const uint8_t *src, size_t inLen, char *dst) noexcept ;

	size_t GetEncodeLen(size_t inLen) noexcept;
#endif

#ifdef WITH_BASE64_DECODING
	size_t DecodeChunk(const char *src, size_t inLen, uint8_t *dst) noexcept ;

	size_t GetDecodeExpectedLen(size_t inLen) noexcept;
#endif
}

#endif /* SRC_GENERAL_BASE64_H_ */
