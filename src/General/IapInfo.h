/* IapInfo.h
 * Shared struct passed from RepRapFirmware to DuetIAP via RAM above the stack.
 * Used for both SD and SBC firmware update modes
 */

#ifndef IAPINFO_H_INCLUDED
#define IAPINFO_H_INCLUDED

#include <cstdint>

struct IapInfo
{
	uint32_t magic;				// MagicValue
	uint32_t auxBaudRate;		// AUX serial baud rate (0 = don't initialise)
	uint8_t transport;			// TransportSd, TransportSpi, or TransportUsb
	char firmwareFilename[];	// null-terminated firmware filename (SD mode only, flexible array)

	static constexpr uint32_t MagicValue = 0x49415049;	// "IAPI"
	static constexpr uint8_t TransportSd = 0;
	static constexpr uint8_t TransportSpi = 1;
	static constexpr uint8_t TransportUsb = 2;
};

#endif // IAPINFO_H_INCLUDED
