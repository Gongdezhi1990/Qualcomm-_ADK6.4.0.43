/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_qce_handler.c

DESCRIPTION
    Functions for handling Qualcommm Codec Extensions AT commands and state.

NOTES

*/

#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_parse.h"
#include "hfp_link_manager.h"
#include "hfp_send_data.h"
#include "hfp_qce_handler.h"

#include <panic.h>
#include <print.h>
#include <stdio.h>
#include <string.h>

/****************************************************************************
NAME
    hfpHandleQacResponse

DESCRIPTION
    Handle Qualcomm Codec Modem IDs supported, received from the AG (Q2Q
    feature). Received in response to AT%QAC from HF in QCE Supported device.

    The AG QCE Codec Mode IDs are stored as a bit map for the link.

AT INDICATION
    +%QAC:[<n>[,<n>[,...<n>]]]

RETURNS
    void
*/
void hfpHandleQacResponse(Task link_ptr, const struct hfpHandleQacResponse *CodecModelist)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;

    uint16 num_of_entries = CodecModelist->codecs.count;
    uint16 idx;
    struct value_hfpHandleQacResponse_codecs codec;

    if(num_of_entries > QCE_MAX_NUM_CODECS)
    {
        num_of_entries = QCE_MAX_NUM_CODECS;
    }

    /* Convert the Codec Mode IDs (uint16 values) into a bit map for internal
     * use.
     */
    link->ag_codec_modes = 0;
    for (idx = 0; idx < num_of_entries; idx++)
    {
        /* Get the next Codec ID. */
        codec = get_hfpHandleQacResponse_codecs(&CodecModelist->codecs, idx);

        link->ag_codec_modes |= hfpConvertCodecModeIdToBit(codec.codec);
    }
}


/****************************************************************************
NAME
    hfpHandleQcsNotification

DESCRIPTION
    Handle Qualcomm Codec Selection received from the AG (Q2Q
    feature). This is an unsolicited notification, which the HF must respond
    to with AT+%QCS=n.

    The AG QCE Codec Mode IDs are stored as a bit map for the link.

AT INDICATION
    +%QCS: <n> \r \n

RETURNS
    void
*/
void hfpHandleQcsNotification(Task link_ptr, const struct hfpHandleQcsNotification *notification)
{
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    char qcs[15];

    /* Create the AT cmd we're sending */
    uint8 char_idx = sprintf(qcs, "AT+%%QCS=");

    /* Conditional check
     * - AG selects QCE Codec Mode ID we don't support.
     * - AG tries to select QCE Codec MODE and HF isn't in QCE Mode
     */
    if (theHfp->hf_codec_modes & hfpConvertCodecModeIdToBit(notification->codec))
    {
        link->qce_codec_mode_id = notification->codec;
        sprintf(&qcs[char_idx], "%d\r", notification->codec);
    }
    else
    {
        link->qce_codec_mode_id = CODEC_MODE_ID_UNSUPPORTED;
        sprintf(&qcs[char_idx], "%d\r", CODEC_MODE_ID_UNSUPPORTED);
    }
    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, (uint16)strlen(qcs), qcs, hfpQcsCmdPending);
}



/****************************************************************************
NAME
    hfpHandleQcsAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+QCS cmd.
    If there is an error, then reset the qce_codec_mode_id for this link,
    indicating that no QCE codec is selected.

RETURNS
    void
*/
void hfpHandleQcsAtAck(hfp_link_data* link, hfp_lib_status status)
{
    if (status != hfp_success)
    {
       link->qce_codec_mode_id = CODEC_MODE_ID_UNSUPPORTED;
    }
}
