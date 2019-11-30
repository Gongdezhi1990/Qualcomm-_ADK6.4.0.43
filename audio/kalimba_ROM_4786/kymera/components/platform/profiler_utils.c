/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  profiler.c
 * \ingroup  platform
 *
 * Profiler C functionality. <br>
 */

#if defined(PROFILER_ON) && !defined(__GNUC__)
/****************************************************************************
Include Files
*/
#include "profiler_c.h"
#include "pmalloc/pl_malloc.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

/****************************************************************************
Public Function Definitions
*/
profiler* create_dynamic_profiler(const char *name, unsigned id)
{
    profiler* new_profiler = zpnew(profiler);

    new_profiler->next = UNINITIALISED_PROFILER;
    new_profiler->name = name;
    new_profiler->id = id;

    return new_profiler;
}



void init_static_profiler(profiler* prof, const char *name, unsigned id)
{
    prof->next = UNINITIALISED_PROFILER;
    prof->name = name;
    prof->id = id;
}



void deregister(profiler *profiler_to_remove)
{
    profiler **profiler_entry = &profiler_list;

    /* Deregister the profiler if it was registered. */
    LOCK_INTERRUPTS;
    while (((*profiler_entry)!=NULL) && ((*profiler_entry)!=UNINITIALISED_PROFILER))
    {
        if ((*profiler_entry) == profiler_to_remove)
        {
           /* Remove the profiler from the list. */
           (*profiler_entry) = (*profiler_entry)->next;
           break;
        }
        profiler_entry = &((*profiler_entry)->next);
    }
    UNLOCK_INTERRUPTS;
}


void deregister_by_name_and_id(const char *name, unsigned id)
{
    profiler **cur_prof= &profiler_list;
    LOCK_INTERRUPTS;
    while ((*cur_prof)!=NULL)
    {
        if (((*cur_prof)->id == id) && ((*cur_prof)->name == name))
        {
            profiler* prof_to_remove = (*cur_prof);

            /* Remove the profiler from the list. */
            (*cur_prof) = (*cur_prof)->next;
            pdelete(prof_to_remove);
            break;

        }
        cur_prof = &((*cur_prof)->next);
    }

    UNLOCK_INTERRUPTS;
}

#endif /* defined(PROFILER_ON) && !defined(__GNUC__) */
