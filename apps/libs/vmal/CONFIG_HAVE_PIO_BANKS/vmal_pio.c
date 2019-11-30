/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    vmal_pio.c

DESCRIPTION
    PIO opperations specific to a platform

NOTES

*/

#include <vmtypes.h>
#include <vmal.h>
#include <pio.h>

#define NUMBER_OF_PIO_BANKS        3
#define NUMBER_OF_DEBOUNCE_GROUPS  2

uint32 VmalPioSetDirection(uint16 bank, uint32 mask, uint32 direction)
{
    return PioSetDir32Bank(bank, mask, direction);
}

uint32 VmalPioSet(uint16 bank, uint32 mask, uint32 bits)
{
    return PioSet32Bank(bank, mask, bits);
}

uint32 VmalPioGet(uint16 bank)
{
    return PioGet32Bank(bank);
}

uint32 VmalPioDebounce(uint16 bank, uint32 mask, uint16 count, uint16 period)
{
    return PioDebounce32Bank(bank, mask, count, period);
}

uint32 VmalPioDebounceGroup(uint16 group, uint16 bank, uint32 mask, uint16 count, uint16 period)
{
    return PioDebounceGroup32Bank(group, bank, mask, count, period);
}

uint32 VmalPioSetMap(uint16 bank, uint32 mask, uint32 bits)
{
    return PioSetMapPins32Bank(bank, mask, bits);
}

uint16 VmalGetNumberOfPioBanks(void)
{
    return NUMBER_OF_PIO_BANKS;
}

uint16 VmalGetNumberOfDebounceGroups(void)
{
    return NUMBER_OF_DEBOUNCE_GROUPS;
}
