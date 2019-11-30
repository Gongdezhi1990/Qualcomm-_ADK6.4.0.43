/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    pio_common_pin_function.c

DESCRIPTION
    Common PIO operations used by libraries and applications
    Enable pins for special functions

NOTES

*/

/****************************************************************************
    Header files
*/
#include <stdlib.h>
#include <pio.h>
#include <print.h>
#include "pio_common.h"

/****************************************************************************
    Public functions
*/
bool PioCommonEnableFunctionPins(pio_common_pin_t pins)
{
    (void) pins;
    return TRUE;
}
