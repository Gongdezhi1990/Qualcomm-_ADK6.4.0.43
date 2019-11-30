/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implements fault database functionality.
 */  

#include "fault/fault_private.h"

faultinfo faultdb[NFI];

/** Locks the database to prevent others from using it. */
#define LOCKDB()   block_interrupts()

/** Unlocks the database to allow others to use it. */
#define UNLOCKDB() unblock_interrupts()


void fault_db_insert(faultid f, DIATRIBE_TYPE arg, bool freshen)
{
    faultinfo *fi, newfi;
    uint16 i;
    TIME now = get_time();

#ifdef PRESERVED_MODULE_PRESENT
    /* Store in the "preserve" ram. */
    preserved.fault.last_id = f;
    preserved.fault.last_arg = (uint16)arg;
#endif

    /* Are we already collecting faults of this type? */
    for (i = 0, fi = faultdb; i < NFI; i++, fi++)
    {
        if (fi->f == f)
        {
            /* There should be nothing to clear the database above us, although
            new stuff may appear there. */
            LOCKDB();
            if (fi->f == f) /* Making sure the entry is still there. */
            {
                if (fi->n < MAX_FI_REPORTS)
                {
                    ++fi->n;
                }
                if (freshen)
                {
                    fi->arg = arg;
                    fi->st = now;
                }
                UNLOCKDB();
                return;
            }
            UNLOCKDB();
        }
    }

    /* We are not yet collecting messages of this type, so try to open a new
       record in faultinfo[], taking care to minimise the time for which we
       block interrupts. */
    newfi.f = f;
    newfi.arg = arg;
    newfi.n = 1;
    newfi.st = now;

    for (i = 0, fi = faultdb; i < NFI; i++, fi++)
    {
        if (fi->f == FAULT_NONE)
        {
            LOCKDB();
            if (fi->f == FAULT_NONE) /* Making sure the entry is still free. */
            {
                *fi = newfi;
                UNLOCKDB();
                return;
            }
            UNLOCKDB();
        }
    }

    /* No room in the database for the fault report.  We used to record the
       overflow fault as an "unknown fault counter", but to save RAM we now
       just discard it. */
}

bool fault_db_dispense(faultinfo *fi)
{
    static uint16 i = 0;

    if(i < NFI)
    {
        *fi = faultdb[i];
        if(faultdb[i].f != FAULT_NONE)
        {
            LOCKDB();
            if(faultdb[i].f != FAULT_NONE) /* Making sure the entry is still there */
            {
                if(faultdb[i].n)
                {
                    /* Service and keep in DB in case new ones arrive. */
                    faultdb[i].n = 0;
                }
                else
                {
                    /* already serviced + no new ones = remove. */
                    faultdb[i].f = FAULT_NONE;
                }
            }
            UNLOCKDB();
        }
        i++;
        return TRUE;
    }
    else
    {
        /* Done looping the database */
        i = 0;
        return FALSE;
    }
}

