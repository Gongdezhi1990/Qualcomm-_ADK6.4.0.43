/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#include "pmalloc/pl_malloc.h"
#include "mem_utils/shared_memory_ids.h"
#include "mem_utils/shared_memory.h"

#include "shared_volume_control.h"


SHARED_VOLUME_CONTROL_OBJ* allocate_shared_volume_cntrl(void)
{
    SHARED_VOLUME_CONTROL_OBJ *vol;
    bool new_allocation = FALSE;
    
    vol = (SHARED_VOLUME_CONTROL_OBJ*)shared_zmalloc(sizeof(SHARED_VOLUME_CONTROL_OBJ),MALLOC_PREFERENCE_NONE,CVC_SHARED_VOLUME_CONTROL,&new_allocation);
    if(vol && new_allocation)
    {
        /* 1.0 in Q5.xx */
        vol->inv_post_gain = 1<<(DAWTH-5);  
    }
    return(vol);
}

void release_shared_volume_cntrl(SHARED_VOLUME_CONTROL_OBJ *vol)
{
    if(vol)
    {
       shared_free(vol); 
    }
}

