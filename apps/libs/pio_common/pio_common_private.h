/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    pio_common_private.h

DESCRIPTION
    Private pio_common functions shared throughout the library.

*/

#ifndef PIO_COMMON_PRIVATE_H_
#define PIO_COMMON_PRIVATE_H_

#include "pio_common.h"

#define NUMBER_OF_PIOS_PER_BANK    (32U)
#define PIO_NONE                   (0UL)
#define PIO_SUCCESS                PIO_NONE

#define PIO_BANK(pio)   (uint16)((pio)/NUMBER_OF_PIOS_PER_BANK)
#define PIO_COMMON_ASSERT(x) PanicNull(x)

/*!
    @brief pioCommonGetMask, get the mask for a given pio

    @return uint32, the mask of the pio
*/
uint32 pioCommonGetMask(uint8 pio);

/*!
    @brief pioCommonSetConvertedFunction, set the function for a given pio

    @return TRUE if function set for pin, FALSE otherwise.
*/
bool pioCommonSetConvertedFunction(uint16 pin, pio_common_pin_function_id function);

#endif /* PIO_COMMON_PRIVATE_H_ */

