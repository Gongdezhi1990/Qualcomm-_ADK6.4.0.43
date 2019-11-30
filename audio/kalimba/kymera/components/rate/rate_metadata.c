/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_metadata.c
 * \ingroup rate_lib
 *
 */

#include "rate_private.h"

#ifdef INSTALL_METADATA

/****************************************************************************
 * Public Function Implementations
 */

RATE_TIMESTAMP_TYPE rate_metadata_get_timestamp_type(
        const metadata_tag* tag)
{
    patch_fn_shared(rate_lib);

    if (tag == NULL)
    {
        return RATE_TIMESTAMP_TYPE_NONE;
    }
    else if (IS_TIMESTAMPED_TAG(tag))
    {
        return RATE_TIMESTAMP_TYPE_TTP;
    }
    else if (IS_TIME_OF_ARRIVAL_TAG(tag))
    {
        return RATE_TIMESTAMP_TYPE_TOA;
    }
    else
    {
        return RATE_TIMESTAMP_TYPE_NONE;
    }
}


#endif /* INSTALL_METADATA */
