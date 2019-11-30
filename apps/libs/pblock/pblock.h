/******************************************************************************
Copyright (c) 2007 - 2015 Qualcomm Technologies International, Ltd.
   
   $Change: 3191847 $  $DateTime: 2019/04/11 15:24:05 $         
   */           
/******************************************************************************/

/******************************************************************************
    MODULE:
        pblock.c

    DESCRIPTION
        This module implements a persistence system called pblock.
        Pblock allows keyed blocks of data to be persisted to/from a PSKey
        Intended usage is for multiple DSP applications running on a chip, each 
        needing an independent block of persistent memory.
  
    Functions:
        void PblockInit(key, len)
            Initialize the PBlock system to use a specific key. 
            Called once per powercycle
  
        void PblockStore(void)
            Store the data to persistance. Can be called at will.
  
        uint16* PblockGet(uint16 entry_id, uint16* entry_len)
            Retrieve data from a keyed block
  
        void PblockSet(uint16 entry_id, uint16 entry_len, uint16 *data)
            Store data to a keyed block. Entry will be created if not present

    Notes
        Storage format:
            {entry header}
            entryid_entrysize:8:8
            {entry data}
            data0:16
            data1:16
            dataN:16
            {entry header}
            entryid_entrysize:8:8
            {entry data}
            data0:16
            data1:16
            dataN:16
            {max_size}

   */  
/******************************************************************************/

#ifndef PBLOCK_H_
#define PBLOCK_H_

#include <vmtypes.h>

#ifdef HYDRACORE
    /* Hydra devices have true octets. If a bitfield was used to pack a byte
     * then two * 8 bits would occupy 32 bits. 
     * Packing maintained for these chips for compatibility, otherwise
     * the key usage would expand by 1 word per pblock_entry */
#define PBLOCK_PACKED_BYTE(field) uint8 field
#else
#define PBLOCK_PACKED_BYTE(field) unsigned field:8
#endif  /* HYDRACORE */


typedef struct
{
    PBLOCK_PACKED_BYTE(id);
    PBLOCK_PACKED_BYTE(size);   /* Note that this is the number of WORDS not bytes */
    uint16   data[1];
} pblock_entry;

/* The code generator used for host testing fails to parse this */
/* STATIC_ASSERT(sizeof(pblock_entry) == 2 * sizeof(uint16),pblock_entry_not_sized_as_expected); */

struct pblock_key_str;
typedef struct pblock_key_str pblock_key;

pblock_key * PblockInit(uint16 key, uint16 len);
void PblockStore(void);
const pblock_entry* PblockGet(uint16 id);
void PblockSet(uint16 id, uint16 len, uint16 *data);

#endif /*PBLOCK_H_*/
