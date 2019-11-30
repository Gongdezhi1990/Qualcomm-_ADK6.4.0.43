/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_qce.c

DESCRIPTION
    API function implementations for Qualcomm Codec Extensions (QCE.

*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"

/****************************************************************************
NAME
    HfpGetSupportedQceCodecModeIds

DESCRIPTION
   Get the current HF supported Codec Mode bitmask.

RETURNS
    uint16 bit mask
*/
uint16 HfpGetSupportedQceCodecModeIds(void)
{
    return theHfp->hf_codec_modes;
}

/****************************************************************************
NAME
    HfpSetSupportedQceCodecModeIds

DESCRIPTION
    If there are currently no SLC connections, and the new bitmask is valid,
    set the new bitmask.

RETURNS
    bool TRUE if successful, otherwise FALSE.
*/
bool HfpSetSupportedQceCodecModeIds(uint16 hf_codec_bitmask)
{
    hfp_link_data    *link;
    uint16           new_codec_modes = hf_codec_bitmask & CODEC_MODE_ID_MASK;

    for_all_links(link)
    {
        if (link->bitfields.ag_slc_state > hfp_slc_disabled)
        {
            return FALSE;
        }
    }

    if (new_codec_modes)
    {
        theHfp->hf_codec_modes = new_codec_modes;
        return TRUE;
    }

    return FALSE;
}


/****************************************************************************
NAME
    HfpGetSupportedQceCodecModeIdsInAg

DESCRIPTION
    If there is a link, and the AG has QCE mode enabled (e.g. a Codec Mode Ids
    bitmask has been defined and exchanged), then return this to the calling
    function.

RETURNS
    uint16 bitmap from AG or CODEC_MODE_ID_UNSUPPORTED.
*/
uint16 HfpGetSupportedQceCodecModeIdsInAg(hfp_link_priority priority)
{

    hfp_link_data* link = hfpGetLinkFromPriority(priority);

    /* if there is no active link, this will be CODEC_MODE_ID_UNSUPPORTED
     * in any case.
     */
    if (link)
    {
        return link->ag_codec_modes;
    }
    else
    {
        return CODEC_MODE_ID_UNSUPPORTED;
    }
}


/****************************************************************************
NAME
    HfpGetQceSelectedCodecModeId

DESCRIPTION
    If there is a link, and the AG has selected a Codec Mode ID on this link,
    then return the selected Codec Mode ID in the pointer.

RETURNS
    TRUE if QCE mode enabled and a codec is selected, otherwise FALSE.
*/
bool HfpGetQceSelectedCodecModeId(hfp_link_priority priority, uint16 *mode_id)
{
    bool            return_value = FALSE;
    hfp_link_data*  link         = hfpGetLinkFromPriority(priority);

    *mode_id = CODEC_MODE_ID_UNSUPPORTED;

    if(link)
    {
        if (link->qce_codec_mode_id != CODEC_MODE_ID_UNSUPPORTED)
        {
            *mode_id = link->qce_codec_mode_id;
            return_value = TRUE;
        }
    }
    return return_value;
}
