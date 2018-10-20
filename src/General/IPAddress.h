/*
 * IPAddress.h
 *
 *  Created on: 16 Oct 2018
 *      Author: David
 */

#ifndef SRC_GENERAL_IPADDRESS_H_
#define SRC_GENERAL_IPADDRESS_H_

#include <cstdint>

// Class to represent an IP address. Currently it only supports IPV4 addresses, but eventually it will be expanded to support IPV6 too.
class IPAddress
{
public:
	constexpr IPAddress() : v4Address(0) {  }
	explicit IPAddress(const uint8_t ip[4]) { SetV4(ip); }

	bool operator==(const IPAddress& other) const { return v4Address == other.v4Address; }
	bool operator!=(const IPAddress& other) const { return v4Address != other.v4Address; }

	IPAddress& operator=(const IPAddress& other) { v4Address = other.v4Address; return *this; }

	bool IsV4() const { return true; }
	bool IsV6() const { return false; }
	uint32_t GetV4LittleEndian() const { return v4Address; }
	uint8_t GetQuad(unsigned int n) const { return (v4Address >> (8 * n)) & 0x00FF; }
	bool IsNull() const { return v4Address == 0; }
	bool IsBroadcast() const { return v4Address == 0xFFFFFFFF; }

	void SetV4LittleEndian(uint32_t ip) { v4Address = ip; }
	void SetV4(const uint8_t ip[4]);
	void SetNull() { v4Address = 0; }
	void SetBroadcast() { v4Address = 0xFFFFFFFF; }

#if 0		// these functions are not currently used
	uint32_t GetV4BigEndian() const { return __builtin_bswap32(v4Address); }
	void SetV4BigEndian(uint32_t ip) { v4Address = __builtin_bswap32(ip); }
#endif

	void UnpackV4(uint8_t rslt[4]) const;

private:
	uint32_t v4Address;
};

#endif /* SRC_GENERAL_IPADDRESS_H_ */
