/*
 * IP4String.h
 *
 *  Created on: 19 Sep 2017
 *      Author: David
 */

#ifndef SRC_LIBRARIES_GENERAL_IP4STRING_H_
#define SRC_LIBRARIES_GENERAL_IP4STRING_H_

#include "../ecv_duet3d.h"
#include <cstdint>
#include "IPAddress.h"

// Class to convert an IPv4 address to a string representation
class IP4String
{
public:
	explicit IP4String(const uint8_t ip[4]) noexcept;
	explicit IP4String(uint32_t ip) noexcept;
	explicit IP4String(IPAddress ip) noexcept : IP4String(ip.GetV4LittleEndian()) {}

	const char *_ecv_array c_str() const noexcept { return buf; }

private:
	char buf[16];		// long enough for e.g. "255.255.255.255" including a null terminator
};

#endif /* SRC_LIBRARIES_GENERAL_IP4STRING_H_ */
