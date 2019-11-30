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

#define NUMBER_OF_PIO_BANKS        1
#define NUMBER_OF_DEBOUNCE_GROUPS  1

uint32 VmalPioSetDirection(uint16 bank, uint32 mask, uint32 direction)
{
    UNUSED(bank);
    return PioSetDir32(mask, direction);
}

uint32 VmalPioSet(uint16 bank, uint32 mask, uint32 bits)
{
    UNUSED(bank);
    return PioSet32(mask, bits);
}

uint32 VmalPioGet(uint16 bank)
{
    UNUSED(bank);
    return PioGet32();
}

uint32 VmalPioDebounce(uint16 bank, uint32 mask, uint16 count, uint16 period)
{
    UNUSED(bank);
    return PioDebounce32(mask, count, period);
}

uint32 VmalPioDebounceGroup(uint16 group, uint16 bank, uint32 mask, uint16 count, uint16 period)
{
    UNUSED(group);
    UNUSED(bank);
    return PioDebounce32(mask, count, period);
}

uint32 VmalPioSetMap(uint16 bank, uint32 mask, uint32 bits)
{
    UNUSED(bank);
    return PioSetMapPins32(mask, bits);
}

uint16 VmalGetNumberOfPioBanks(void)
{
    return NUMBER_OF_PIO_BANKS;
}

uint16 VmalGetNumberOfDebounceGroups(void)
{
    return NUMBER_OF_DEBOUNCE_GROUPS;
}
