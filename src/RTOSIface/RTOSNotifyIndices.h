/*
 * RTOSNotifyIndices.h
 *
 *  Created on: 2 Jan 2024
 *      Author: David
 *
 *  Definitions of task notification indices used by the RTOS interface layer
 */

#ifndef SRC_RTOSIFACE_RTOSNOTIFYINDICES_H_
#define SRC_RTOSIFACE_RTOSNOTIFYINDICES_H_

#include <cstdint>

namespace NotifyIndices
{
	constexpr uint32_t ReadWriteLocker = 0;
	constexpr uint32_t NextAvailableAfterRTOS = 1;
}

#endif /* SRC_RTOSIFACE_RTOSNOTIFYINDICES_H_ */
