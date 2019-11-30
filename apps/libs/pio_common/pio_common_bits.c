/****************************************************************************
Copyright (c) 017 Qualcomm Technologies International, Ltd.

FILE NAME
    pio_common_bits.c

DESCRIPTION
    Common bit manipulation operations used by libraries and applications

NOTES

*/
#include <vmtypes.h>
#include <vmal.h>
#include <string.h>
#include <panic.h>

#include "pio_common_private.h"
#include "pio_common.h"

/***************************************************************************/
void PioCommonBitsInit(pio_common_allbits *mask)
{
    PIO_COMMON_ASSERT(mask);
    memset(mask,0,sizeof(*mask));
}

/***************************************************************************/
void PioCommonBitsSetBit(pio_common_allbits *mask, uint8 pio)
{
    uint16 bank = PIO_BANK(pio);

    PIO_COMMON_ASSERT(mask);

    mask->mask[bank] |= pioCommonGetMask(pio);
}

/***************************************************************************/
bool PioCommonBitsBitIsSet(pio_common_allbits *mask, uint8 pio)
{
    uint32 bitmask = pioCommonGetMask(pio);
    uint16 bank = PIO_BANK(pio);
    bool isBitSet = FALSE;

    PIO_COMMON_ASSERT(mask);

    if (mask->mask[bank] & bitmask)
    {
        isBitSet = TRUE;
    }

    return isBitSet;
}

/***************************************************************************/
bool PioCommonBitsIsEmpty(pio_common_allbits *mask)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();
    bool areAllBitsCleared = TRUE;

    PIO_COMMON_ASSERT(mask);

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        if (mask->mask[bank] != 0)
        {
            areAllBitsCleared = FALSE;
        }
    }

    return areAllBitsCleared;
}

/***************************************************************************/
bool PioCommonBitsOR(pio_common_allbits *result,
                     pio_common_allbits *mask1,
                     pio_common_allbits *mask2)
{
    uint32 anyset = 0;
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(result);
    PIO_COMMON_ASSERT(mask1);
    PIO_COMMON_ASSERT(mask2);

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        result->mask[bank] = mask1->mask[bank] | mask2->mask[bank];
        anyset |= result->mask[bank];
    }

    return anyset != 0;
}

/***************************************************************************/
bool PioCommonBitsXOR(pio_common_allbits *result,
                      pio_common_allbits *mask1,
                      pio_common_allbits *mask2)
{
    uint32 changed = 0;
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(result);
    PIO_COMMON_ASSERT(mask1);
    PIO_COMMON_ASSERT(mask2);

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        result->mask[bank] = mask1->mask[bank] ^ mask2->mask[bank];
        changed |= result->mask[bank];
    }

    return changed != 0;
}

/***************************************************************************/
bool PioCommonBitsAND(pio_common_allbits *result,
                      pio_common_allbits *mask1,
                      pio_common_allbits *mask2)
{
    uint32 anyset = 0;
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(result);
    PIO_COMMON_ASSERT(mask1);
    PIO_COMMON_ASSERT(mask2);

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        result->mask[bank] = mask1->mask[bank] & mask2->mask[bank];
        anyset |= result->mask[bank];
    }

    return anyset != 0;
}

/***************************************************************************/
void PioCommonBitsRead(pio_common_allbits *pioState)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    PIO_COMMON_ASSERT(pioState);

    PioCommonBitsInit(pioState);

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        pioState->mask[bank] = VmalPioGet(bank);
    }
}

void PioCommonSetDir(pio_common_allbits mask, pio_common_allbits direction)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        VmalPioSetDirection(bank, mask.mask[bank], direction.mask[bank]);
    }
}

void PioCommonSet(pio_common_allbits mask, pio_common_allbits bits)
{
    uint16 bank;
    uint16 number_of_banks = VmalGetNumberOfPioBanks();

    for (bank = 0; bank < number_of_banks; bank ++)
    {
        VmalPioSet(bank, mask.mask[bank], bits.mask[bank]);
    }
}
