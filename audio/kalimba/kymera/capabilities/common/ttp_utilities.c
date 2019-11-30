/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  ttp_utilities.c
 * \ingroup  capabilities
 *
 *  Common functions
 *  Used by capabilities to manage time-to-play information in metadata
 */


#include "ttp_utilities.h"
#include "platform/pl_assert.h"

#include "patch/patch.h"

#ifdef INSTALL_TTP

/**
 * ttp_utils_populate_tag
 */
void ttp_utils_populate_tag(metadata_tag *tag, ttp_status *status)
{
    patch_fn(ttp_utils_populate_tag);       
    
    PL_ASSERT(tag != NULL);
    PL_ASSERT(status != NULL);
    METADATA_TIMESTAMP_SET(tag, status->ttp, METADATA_TIMESTAMP_LOCAL);
    tag->sp_adjust = status->sp_adjustment;
    if (status->err_offset_id != INFO_ID_INVALID)
    {
        /* Add the error offset to the "private" data for this tag */
        (void)buff_metadata_add_private_data(tag, META_PRIV_KEY_TTP_OFFSET, sizeof(unsigned), &status->err_offset_id);
    }
    if (status->stream_restart)
    {
        METADATA_STREAM_START_SET(tag);
    }
}

#endif /* INSTALL_TTP */
