/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef PORTABILITY_H_
#define PORTABILITY_H_

#include "trap_api/trap_api.h"

/**
 * Init USB device descriptors
 *
 * Called before the rest of the firmware to configure
 * USB device descriptors. */
extern void _init(void);

#define PioDebounce32(mask, count, period) PioDebounce32Bank(0, mask, count, period)
#define PioGet32() PioGet32Bank(0)
#define PioSet32(mask, bits) PioSet32Bank(0, mask, bits)
#define PioGetDir32() PioGetDir32Bank(0)
#define PioSetDir32(mask, dir) PioSetDir32Bank(0, mask, dir)
#define PioGetStrongBias32() PioGetStrongBias32Bank(0)
#define PioSetStrongBias32(mask, bits) PioSetStrongBias32Bank(0, mask, bits)
#define PioGetMapPins32() PioGetMapPins32Bank(0)
#define PioSetMapPins32(mask, bits) PioSetMapPins32Bank(0, mask, bits)
#define PioGetUnusedPins32() PioGetUnusedPins32Bank(0)

/*@}*/

#endif /* PORTABILITY_H_ */

