/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "trap_api/trap_api_private.h"


int UtilCompareByte(const uint8 *a, const uint8 *b, size_t size)
{
    return memcmp(a, b, size * sizeof(uint8));
}

int UtilCompareWord(const uint16 *a, const uint16 *b, size_t size)
{
    return memcmp(a, b, size * sizeof(uint16));
}

uint8 *UtilFindByte(uint8 mask, uint8 value, const uint8 *data_start, uint16 offset, uint16 size, uint16 count)
{
    /* If we imagine a table in memory of "count" entries, each "size" large,
     * beginning at "data_start". We search for "value" at "offset" from the
     * beginning of each entry using bitmask "mask". We return the entry we
     * find
     * "value" at, else 0*/
    uint16 i;
    for (i=0; i<count; i++)
    {
        if ((*(data_start + offset + (i*size)) & mask) == value)
        {
            return (uint8 *)(size_t)(data_start + (i*size));
        }
    }
    return (void *)0;
}

uint16 *UtilFindWord(uint16 mask, uint16 value, const uint16 *data_start, uint16 offset, uint16 size, uint16 count)
{
    /* If we imagine a table in memory of "count" entries, each "size" large, 
     * beginning at "data_start". We search for "value" at "offset" from the 
     * beginning of each entry using bitmask "mask". We return the entry we
     * find 
     * "value" at, else 0*/
    uint16 i;




    for (i=0; i<count; i++)
    {
        if ((*(data_start + offset + (i*size)) & mask) == value)
        {
            return (uint16 *)(size_t)(data_start + (i*size));
        }
    }
    return (void *)0;
}

uint8 *UtilGetNumber(const uint8 *start, const uint8 *end, uint16 *result)
{
    if(start)
    {
        const uint8 *p = start;
        uint32 r = 0;
        while( p != end )
        {
            uint16 c = *p;
            if (!((c >= '0') && (c <= '9')))
            {
                break;
            }                        
            r = (r * 10) + (uint16)(c - '0');
            if (r > 0xffff)
            {
                r = 0;
                p = start;
                break;
            }
            p++;
        }
        if (result)
        {
            *result = (uint16)r;
        }
        return p != start ? (uint8 *)(size_t)p : (void *)0;
    }
    return (void *)0;
}

uint8 UtilHashByte(const uint8 *data, uint16 size, uint8 seed)
{
    const uint8 MULTIPLIER = 157U;
    const uint8 *block;
    uint16 count, i = 0, j;

#ifdef INSTALL_VM_STREAM_SUPPORT
    /*lint -e{774} : this is if(FALSE) for now */
    if(IS_STREAM(data))
        count = 1;
    else
#endif  /* INSTALL_VM_STREAM_SUPPORT */
        count = size;
    while(i < size)
    {
        block = data + i;
        /*lint -e{774} : lint thinks block is never NULL */
        if(block == NULL)
        {
            break;
        }
        for(j = 0; j < count; j++)
        {
            seed = (uint8)((seed + *block++) * MULTIPLIER);
        }
        i += count;
    }
    return seed;
}

uint16 UtilHashWord(const uint16 *data, uint16 size, uint16 seed)
{
    const uint16 MULTIPLIER = 40503U;
    const uint16 *block;
    uint16 count, i = 0, j;

#ifdef INSTALL_VM_STREAM_SUPPORT
    /*lint -e{774} : this is if(FALSE) for now */
    if(IS_STREAM(data))
        count = 1;
    else
#endif  /* INSTALL_VM_STREAM_SUPPORT */
        count = size;
    while(i < size)
    {
        block = data + i;
        /*lint -e{774} : lint thinks block is never NULL */
        if(block == NULL)
        {
            break;
        }
        for(j = 0; j < count; j++)
        {
            seed = (uint16)((seed + *block++) * MULTIPLIER);
        }
        i += count;
    }
    return seed;
}

uint16 UtilHash(const uint8 *data, uint16 size_bytes, uint16 seed)
{
    const uint16 MULTIPLIER = 40503U;
    const uint8 *block;
    uint16 count, i = 0, j;
    uint16 size = size_bytes;

#ifdef INSTALL_VM_STREAM_SUPPORT
    /*lint -e{774} : this is if(FALSE) for now */
    if(IS_STREAM(data))
        count = 1;
    else
#endif  /* INSTALL_VM_STREAM_SUPPORT */
        count = size;
    while(i < size)
    {
        block = data + i;
        /*lint -e{774} : lint thinks block is never NULL */
        if(block == NULL)
        {
            break;
        }
        for(j = 0; j < count; j++)
        {
            seed = (uint16)((seed + *block++) * MULTIPLIER);
        }
        i += count;
    }
    return seed;
}

void UtilSwap(uint16 *data, uint16 size)
{
    while(size--)
    {
        *data = (uint16)(((*data << 8) & 0xff00) | ((*data >> 8) & 0x00ff));
        data++;
    }
}

