/******************************************************************************
Copyright (c) 2007 - 2015 Qualcomm Technologies International, Ltd.
   
   $Change: 641757 $  $DateTime: 2010/10/20 20:54:35 $         
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
         Initialize the PBlock system. Call at most ONCE per powercycle 
         specifying the key to be used. Do not call again.
  
      void PblockStore(void)
         Store the data to persistance. Can be called at will.
  
      uint16* PblockGet(uint16 entry_id, uint16* entry_len)
         Retrieve data from a keyed block
  
      void PblockSet(uint16 entry_id, uint16 entry_len, uint16 *data)
         Store data to a keyed block. Entry will be created if not present

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

   Notes
   */  
/******************************************************************************/

#include <ps.h>
#include <stdlib.h>
#include <string.h>
#include <panic.h>
#include <print.h>
#include <stddef.h>

#include "pblock.h"

struct pblock_key_str
{
    uint16      key;
    unsigned    allocated_len_words:8;
    unsigned    used_len_words:8;       /* Includes header for each block */
    uint16      cache[1];
};

#define PBLOCK_START    ((pblock_entry*)&pblock->cache)
#define PBLOCK_END      ((pblock_entry*)(&pblock->cache + pblock->used_len_words))
#define for_all_entries(entry)      for(entry = PBLOCK_START; entry < PBLOCK_END; entry = (pblock_entry*)(((uint16*) entry) + entry->size + ENTRY_HDR_SIZE_WORDS))
#define for_all_data(entry, data)   for(data = entry->data; data < (entry->data + entry->size); data++)

#define ENTRY_HDR_SIZE          (offsetof(pblock_entry,data))
#define ENTRY_HDR_SIZE_WORDS    ((ENTRY_HDR_SIZE + sizeof(uint16) - 1)/ sizeof(uint16))

#define PBLOCK_HDR_SIZE         (offsetof(pblock_key,cache))

STATIC_ASSERT(ENTRY_HDR_SIZE == sizeof(uint16),block_hdr_isnt_a_word);

static pblock_key* pblock;

static const pblock_entry empty_pblock = {0, 0, {0}};

#ifdef DEBUG_PRINT_ENABLED
static void pblockDebug(void)
{
    pblock_entry* entry;
    uint16*       data;
    
    for_all_entries(entry)
    {
        PRINT(("PBLOCK: Entry 0x%02X:", entry->id));
        for_all_data(entry, data)
        {
            PRINT((" 0x%04X", *data));
        }
        PRINT(("\n"));
    }
}
#else
#define pblockDebug() ((void)(0))
#endif

static pblock_entry* pblockAddEntry(uint16 id, uint16 len_words)
{
    pblock_entry* entry;
    uint16 new_used_len_words;
   
    /* ensure pblock has been initialised */
    if (!pblock)
        return NULL;

    new_used_len_words = pblock->used_len_words + len_words + ENTRY_HDR_SIZE_WORDS;

    /* if not enough memory is reserved, increase the malloc'd memory for the key */
    if (new_used_len_words > pblock->allocated_len_words)
    {
        pblock_key* pblock_temp;
        
        /* add another block of memory and realloc the pblock cache size */
        pblock_temp = (pblock_key*)realloc(pblock,PBLOCK_HDR_SIZE + new_used_len_words * sizeof(uint16)); 

        /* ensure memory is available */
        if(pblock_temp)
        {
            pblock = pblock_temp;
            pblock->allocated_len_words = new_used_len_words;
        }
        /* out of memory, extra space not available */
        else 
        {
            PRINT(("PBLOCK: New Entry FAILED, out of memory 0x%02x (size=%d)\n", id, len_words));
            return NULL;
        }
    }

    /* Need to assign here as block may have moved */
    entry = PBLOCK_END;

    PRINT(("PBLOCK: New Entry 0x%02x (size=%d)\n", id, len_words));

    entry->id = id;
    entry->size = len_words;
    pblock->used_len_words = new_used_len_words;
   
    return entry;
}

pblock_key * PblockInit(uint16 key, uint16 len_words)
{
    /* determine if pskey exists and what size it is */
    uint16 buf_len_words = PsRetrieve(key, NULL, 0);
    /* allow an extra word for the header of a block len_words long */
    uint16 variable_length = buf_len_words + len_words + ENTRY_HDR_SIZE_WORDS;

    /* malloc block for key and cache depending upon whether the pskey currently
       exists and what size it is, will be realloc'd later if required */
    pblock = (pblock_key*)PanicUnlessMalloc(PBLOCK_HDR_SIZE + variable_length * sizeof(uint16));
    
    /* initialise pskey id and length */
    pblock->key = key;
    pblock->allocated_len_words = variable_length;
        
    PRINT(("PBLOCK: Load\n"));
    /* if pskey exists read in data */
    if (buf_len_words)
    {
        pblock->used_len_words = PsRetrieve(pblock->key, PBLOCK_START, pblock->allocated_len_words);
    }
    else
    {
        /* otherwise current length of cache is 0 */    
        pblock->used_len_words = 0;
    }
            
    pblockDebug();
    
    return pblock;
}

void PblockStore(void)
{
    PRINT(("PBLOCK: Store\n"));
    PanicFalse(pblock->used_len_words == PsStore(pblock->key, &pblock->cache, pblock->used_len_words));
    pblockDebug();
}

const pblock_entry* PblockGet(uint16 id)
{
    pblock_entry* entry;

    if(pblock)
    {
        for_all_entries(entry)
        {
            if(entry->id == id)
            {
                return entry;
            }
        }
    }

    return &empty_pblock;
}

void PblockSet(uint16 id, uint16 len_words, uint16 *data)
{
    pblock_entry* entry = (pblock_entry*)PblockGet(id);

    if(!entry->size) 
        entry = pblockAddEntry(id, len_words);
    
    if(entry && (entry->size == len_words))
    {
        memmove(entry->data, data, entry->size * sizeof(uint16));
        PRINT(("PBLOCK: Stored %d words to entry 0x%02x \n", len_words, id ));
    }
}
