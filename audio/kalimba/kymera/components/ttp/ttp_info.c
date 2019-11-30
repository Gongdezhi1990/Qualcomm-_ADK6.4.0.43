/**
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 * 
 * \file  ttp_info.c
 *
 * \ingroup ttp
 *
 * Time-to-play (TTP) information broker
 *
 * This provides a mechanism for supplying pointers in metadata, 
 * in a lifetime-robust way. 
 * 
 * The initial requirement for this is TTP startup offset correction,
 * but it may have other applications.
 */

/****************************************************************************
Include Files
*/

#include "ttp_private.h"

/****************************************************************************
Private Type Declarations
*/

typedef struct
{
    void *data;
    unsigned id;
} ttp_info_entry;

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

#define TTP_INFO_SIZE 10

/* No. of bits in the ID used for the entry number
 * 1<<ENTRY_COUNT_BITS must be >= TTP_INFO_SIZE 
 */
#define ENTRY_COUNT_BITS 4
#define ENTRY_COUNT_MASK ((1<<ENTRY_COUNT_BITS)-1)

/****************************************************************************
Private Variable Definitions
*/

static ttp_info_entry ttp_info[TTP_INFO_SIZE];

static unsigned last_info_id;

/****************************************************************************
Public Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

/****************************************************************************
Private Function Definitions
*/


/****************************************************************************
Public Function Definitions
*/

/**
 * \brief  Create a new info entry
 */
unsigned ttp_info_create(void *data)
{
    int entry;
    for (entry = 0; entry < TTP_INFO_SIZE; entry++)
    {
        if (ttp_info[entry].id == INFO_ID_INVALID)
        {
            /* Free entry found */
            ttp_info[entry].id = entry | (++last_info_id << ENTRY_COUNT_BITS);
            ttp_info[entry].data = data;
            return ttp_info[entry].id;
        }
    }

    /* No free entry. The caller *might* be able to continue, 
     * so don't just panic here 
     */
    return INFO_ID_INVALID;
}

/**
 * \brief  Get data pointer for info entry
 */
void *ttp_info_get(unsigned id)
{
    unsigned entry = id & ENTRY_COUNT_MASK;

    if (ttp_info[entry].id == id)
    {
        /* ID matches, so this is still valid */
        return ttp_info[entry].data;
    }
    else
    {
        /* ID doesn't match, no longer valid for this entry */
        return NULL;
    }
}

/**
 * \brief  Destroy info entry
 */
bool ttp_info_destroy(unsigned id)
{
    unsigned entry = id & ENTRY_COUNT_MASK;
    unsigned entry_id = id >> ENTRY_COUNT_BITS;

    if (ttp_info[entry].id == entry_id)
    {
        /* ID matches, so kill this entry*/
        ttp_info[entry].data = NULL;
        ttp_info[entry].id = INFO_ID_INVALID;
        return TRUE; 
    }
    else
    {
        /* ID doesn't match, no longer valid for this entry */
        return FALSE;
    }
}

