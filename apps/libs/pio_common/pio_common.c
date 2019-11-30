/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    pio_common.c

DESCRIPTION
    Common PIO operations used by libraries and applications

NOTES

*/
#include <vmtypes.h>
#include <vmal.h>
#include <print.h>
#include <string.h>
#include <panic.h>
#include <pio.h>

#include "pio_common_private.h"
#include "pio_common.h"

pio_common_allbits invert_mask;

#ifdef DEBUG_PRINT_ENABLED
static void PioCommonAllBitsDebug(pio_common_allbits *pio_mask)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    for (bank = 0; bank < number_of_banks; bank++)
        PRINT(("%08lx ", (unsigned long) pio_mask->mask[bank]));

    PRINT(("\n"));
}
#else
#define PioCommonAllBitsDebug(pio_mask) ((void)(0))
#endif

/****************************************************************************
NAME
    pioCommonSetDirection
    
DESCRIPTION
    This function sets the direction and returns the mask for a given pio
*/
static uint32 pioCommonSetDirection(uint8 pio, pio_common_dir dir)
{
    uint32 mask = pioCommonGetMask(pio);

    if (VmalPioSetDirection(PIO_BANK(pio), mask, (dir ? mask : 0)) != PIO_SUCCESS)
    {
        return PIO_NONE;
    }

    return mask;
}

uint32 pioCommonGetMask(uint8 pio)
{
    if(pioCommonCheckValid(pio))
        return ((uint32)1 << ((pio) & 0x1F));
    return PIO_NONE;
}

/***************************************************************************/
bool pioCommonCheckValid(uint8 pio)
{
    unsigned number_of_pios = NUMBER_OF_PIOS_PER_BANK * VmalGetNumberOfPioBanks();
    return pio < number_of_pios;
}

/***************************************************************************/
bool PioCommonSetPio(uint8 pio, pio_common_dir direction, bool level)
{
    uint32 mask = pioCommonSetDirection(pio, direction);
    bool successful = FALSE;

    PRINT(("PIO: %s %d (0x%lX) ", (direction ? "Drive" : "Pull"), pio, mask));

    level = PioCommonBitsBitIsSet(&invert_mask, pio) ? !level : level;

    if((mask != PIO_NONE) && (VmalPioSet(PIO_BANK(pio), mask, (level ? mask : 0)) == PIO_NONE))
    {
        PRINT(("%s\n", level ? "High" : "Low"));
        successful = TRUE;
    }

    return successful;
}


/***************************************************************************/
bool PioCommonGetPio(uint8 pio)
{
    uint32 mask = pioCommonSetDirection(pio, pio_input);

    mask &= VmalPioGet(PIO_BANK(pio));
    PRINT(("%s\n", mask ? "High" : "Low"));

    return ((mask != PIO_NONE) ? TRUE : FALSE);
}


/***************************************************************************/
bool PioCommonDebounce( pio_common_allbits *pio_mask,
                        uint16 count,
                        uint16 period)
{
    pio_common_allbits  allowed_mask;
    uint16              bank;
    uint16              number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(pio_mask);

    allowed_mask = *pio_mask;

    PioCommonAllBitsDebug(&allowed_mask);

    for (bank = 0; bank < number_of_banks; bank++)
    {
        VmalPioSetDirection(bank,allowed_mask.mask[bank], pio_input);

        if (VmalPioDebounce(bank, allowed_mask.mask[bank], count, period) != PIO_SUCCESS)
            return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
bool PioCommonDebounceGroup(uint16             group,
                            pio_common_allbits *pio_mask,
                            uint16             count,
                            uint16             period)
{
    pio_common_allbits allowed_mask;

    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(pio_mask);
    
    if(group < VmalGetNumberOfDebounceGroups())
    {
        allowed_mask = *pio_mask;

        PioCommonAllBitsDebug(&allowed_mask);

        for(bank = 0; bank < number_of_banks;++bank)
        {
            VmalPioSetDirection(bank, allowed_mask.mask[bank], pio_input);

            if(VmalPioDebounceGroup(group,
                                    bank,
                                    allowed_mask.mask[bank],
                                    count,
                                    period) != PIO_SUCCESS)
                return FALSE;
        }
    }

    return TRUE;
}

/***************************************************************************/
bool PioCommonSetMap(pio_common_allbits *mask,
                     pio_common_allbits *bits)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(mask);
    PIO_COMMON_ASSERT(bits);

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        if (VmalPioSetMap(bank, mask->mask[bank], bits->mask[bank]) != PIO_SUCCESS)
            return FALSE;
    }

    return TRUE;
}

/***************************************************************************/
void PioCommonXORWithInvertMask(pio_common_allbits *pioState)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(pioState);

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        pioState->mask[bank] = pioState->mask[bank] ^ invert_mask.mask[bank];
    }
}

/***************************************************************************/
void PioCommonStoreInvertMask(pio_common_allbits *pioInvertMask)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(pioInvertMask);

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        invert_mask.mask[bank] = pioInvertMask->mask[bank];
    }
}

/***************************************************************************/
bool PioCommonSetFunction(uint16 pin, pio_common_pin_function_id function)
{
    return pioCommonSetConvertedFunction(pin, function);
}
