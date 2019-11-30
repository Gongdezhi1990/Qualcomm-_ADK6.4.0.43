/****************************************************************************
Copyright (c) 2018 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
	aghfp_qce_handler.c

DESCRIPTION
    Functions for handling Qualcommm Codec Extensions AT commands and state.	

NOTES

*/


/****************************************************************************
	Header files
*/
#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_common.h"
#include "aghfp_qce_handler.h"
#include "aghfp_send_data.h"
#include "aghfp_ok.h"

#include <panic.h>
#include <stdio.h>
#include <string.h>


/* eSCO parameter set Q0. Tx/Rx 64 kbps. Codec Mode ID 0 and 4. */
static const aghfp_audio_params esco_parameter_set_q0 =
{
    8000,                           /* Tx and Rx Bandwidth */
    0x000E,                         /* Maximum latency */
    sync_air_coding_transparent,    /* Voice Settings - not relevant*/
    sync_retx_link_quality          /* 2 retransmissions */
};

/* eSCO parameter set Q1. Tx/Rx 128 kbps. Codec Mode ID 6. */
static const aghfp_audio_params esco_parameter_set_q1 =
{
    16000,                          /* Tx and Rx Bandwidth */
    0x00E,                          /* Maximum latency */
    sync_air_coding_transparent,    /* Voice Settings - not relevant*/
    sync_retx_link_quality          /* 2 retransmissions */
};

/* eSCO parameter set Q2. Tx/Rx 64 kbps. Codec Mode ID 7. */
static const aghfp_audio_params esco_parameter_set_q2 =
{
    8000,                           /* Tx and Rx Bandwidth */
    0x0013,                         /* Maximum latency */
    sync_air_coding_transparent,    /* Voice Settings - not relevant*/
    sync_retx_link_quality          /* 2 retransmissions */
};

/****************************************************************************
NAME	
        aghfpHandleHfCodecModes

DESCRIPTION
        Store the Qualcomm Codec Mode IDs sent from the HF in the AT%QAC
        command for comparison later and respond with a list of Qualcomm Codec
        Mode IDs that the AG supports.

RETURNS
	void
*/
void aghfpHandleHfCodecModes(AGHFP *aghfp, AGHFP_INTERNAL_AT_QAC_REQ_T *codec_modes)
{
    aghfp->hf_codec_modes = codec_modes->codec_mode_ids;

    AGHFP_DEBUG(("aghfpHandleHfCodecModes\n"));

    /* If the AG hasn't had any Codec Mode IDs set during initialisation, or
     * they don't match the allowed codec mask, then it is not in QCE mode and
     * the AG should send an ERROR code.
     */

    if ( !(aghfp->ag_codec_modes & CODEC_MODE_ID_MASK) )
    {
        AGHFP_DEBUG(("AG is not in Qualcomm Codec Extensions mode.\n"));
        AghfpSendError(aghfp);
    }
    /* Otherwise, AG is in QCE mode
     * Send OK and respond with the AG list of supported codes with +%QAC. 
     */
    else  
    {
        char codec_mode_id[10];
        int bit;
        uint16 ag_codec_modes =  aghfp->ag_codec_modes;
        
        AGHFP_DEBUG(("AG is in Qualcomm Codec Extensions mode Send OK and +QAC.\n"));

        aghfpSendOk(aghfp);

        /* Construct and send the "+%QAC:[<n>[,<n>[,...<n>]]] AT response. */
        aghfpAtCmdBegin(aghfp);
        aghfpAtCmdString(aghfp, "+%QAC: ");
        
        /* For every bit set in the AG Codec Modes bit field. */
        for (bit=0; bit<16 && ag_codec_modes; bit++) 
        {
            uint16 bit_mask = 1 << bit;

            /* If the codec bit isn't set, go to the next one. */
            if ( !(bit_mask & ag_codec_modes) )
                continue;

            sprintf(
                    codec_mode_id, 
                    "%d",
                    aghfpConvertCodecModeBitToId(bit_mask)
                   );
            aghfpAtCmdString(aghfp, codec_mode_id);

            ag_codec_modes ^= bit_mask;

            /* Are there still more Codec Mode IDs (bits) to process? 
             * Add a comma before the next Codec Mode ID.
             */
            if (ag_codec_modes)
            {
                aghfpAtCmdString(aghfp, ",");
            }
        }
        aghfpAtCmdEnd(aghfp);
    }   
}

/****************************************************************************
NAME	
        aghfpQceSendCodecSelection

DESCRIPTION
        Send the ~%BCS:<n> unsolicited notification with the selected codec.

RETURNS
	void
*/
void aghfpQceSendCodecSelection(AGHFP *aghfp, codec_mode_bit_type codec_mode_bit)
{
    char codec_mode_id[10];

    AGHFP_DEBUG(("AG in QCE Mode - send Qualcomm Codec Selection +QCS\n"));
    
    /* Construct and sent the +%QAC:<n> notification. */
    aghfpAtCmdBegin(aghfp);
    aghfpAtCmdString(aghfp, "+%QCS: ");
    sprintf(codec_mode_id, "%d", aghfpConvertCodecModeBitToId(codec_mode_bit));
    aghfpAtCmdString(aghfp, codec_mode_id);
    aghfpAtCmdEnd(aghfp);

    AGHFP_DEBUG(("Qualcom Codec Selection (%s) sent\n", codec_mode_id));
}

/****************************************************************************
NAME
        aghfpGetQceEscoParams

DESCRIPTION
        Return the QCE eSCO parameters based on the Codec Mode ID selected
        (as a bitfield).

RETURNS
    aghfp_audio_params
*/
const aghfp_audio_params *aghfpGetQceEscoParams(codec_mode_bit_type codec_mode_bit)
{
    const aghfp_audio_params *audio_params = NULL;

    switch (codec_mode_bit)
    {
        case CODEC_64_2_EV3:
            /* FALLTHROUGH */
        case CODEC_64_2_EV3_QHS3:
            audio_params = &esco_parameter_set_q0; /* defaulted */
            break;
        case CODEC_128_QHS3:
            audio_params = &esco_parameter_set_q1;
            break;
        case CODEC_64_QHS3:
            audio_params = &esco_parameter_set_q2;
            break;
        default:
            AGHFP_DEBUG(("codec_mode_bit 0x%04x invalid\n", codec_mode_bit));
            Panic();
        break;
    }
    return audio_params;
}


/****************************************************************************
NAME	
        aghfpHandleHfCodecModes

DESCRIPTION
        Store the Qualcomm Codec Mode IDs sent from the HF in the AT%QAC
        command for comparison later and respond with a list of Qualcomm Codec
        Mode IDs that the AG supports.

RETURNS
	void
*/
void aghfpHandleHfCodecSelect(AGHFP *aghfp, AGHFP_INTERNAL_AT_QCS_REQ_T *codec_select)
{

    codec_mode_bit_type codec_mode_bit = aghfpConvertCodecModeIdToBit(
            codec_select->codec_mode_id
            );

    /* If 
     * - the AG isn't in QCE Mode
     * - receives the mode as special value 0xFFFF (special value)
     * - The selected mode is not one the AG supports 
     * reject the AT+QCS command.
     *
     * NOTE: the conditional below meets all the above requirements.
     * - AG isn't in QCE mode ag_codec_modes is 0.
     * - Special value 0xFFFF, above Convertion function returns 0.
     * - Selected mode is not one the AG supports - no bit match.
     */
    if (!(aghfp->ag_codec_modes & codec_mode_bit))
    {
        AGHFP_DEBUG((
                    "AG received Codec Mode ID %d - not supported.\n",
                    codec_select->codec_mode_id
                    ));
        aghfp->qce_codec_mode_id  = CODEC_MODE_ID_UNSUPPORTED;
        AghfpSendError(aghfp);
    }
    /* Otherwise, AG is in QCE mode and the selected mode IS supported. 
     * - Setup the Audio connection parameters
     */
    else 
    {
        MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AUDIO_CONNECT_REQ);
        
        /* Send OK to HF to indicate AT command exchange success. */
        aghfpSendOk(aghfp);
        
        /* Equivalent to 0x00380. */
        message->packet_type = QCE_MODE_PACKET_TYPE;
        
        message->audio_params = *aghfpGetQceEscoParams(codec_mode_bit);

        /* Store the selected Codec Mode ID for later reference. */
        aghfp->qce_codec_mode_id  = codec_select->codec_mode_id;

        /* Start Audio now that the Audio params and Qualcom Codec Mode ID are
         * selected.
         */
        MessageSend(&aghfp->task, AGHFP_INTERNAL_AUDIO_CONNECT_REQ, message);
    }
}


/****************************************************************************
NAME	
        AghfpGetSupportedQceCodecModeIds

DESCRIPTION
        Returns a bitmask of the supported Codec Mode IDs (see 
        codec_mode_bit_type in library.h) or #CODEC_MODE_ID_UNSUPPORTED if
        none have been configured. 

RETURNS
	uint16 bitmask.
*/
uint16 AghfpGetSupportedQceCodecModeIds(AGHFP *aghfp)
{
    return (aghfp->ag_codec_modes) ? aghfp->ag_codec_modes : CODEC_MODE_ID_UNSUPPORTED;
}


/****************************************************************************
NAME	
        AghfpSetSupportedQceCodecModeIds

DESCRIPTION
        Set the AG supported Codec Mode IDS bit mask. 

RETURNS
	TRUE if successful, otherwise FALSE.
*/
bool AghfpSetSupportedQceCodecModeIds(AGHFP *aghfp, uint16 ag_codecs_bitmask)
{
    bool local_success = FALSE;

    if (ag_codecs_bitmask & CODEC_MODE_ID_MASK)
    {
        if (aghfp->state == aghfp_ready)
        {
            aghfp->ag_codec_modes = ag_codecs_bitmask;
            local_success = TRUE;
        }
    }
    return local_success;
}


/****************************************************************************
NAME	
        AghfpGetSupportedQceCodecModeIdsInHF

DESCRIPTION
        If an SLC has been established and the HF and AG both support QCE mode,
        the bitmap of the HF mode IDs is put into mode_ids pointer, otherwise
        sets the value to CODEC_MODE_ID_UNSUPPORTED.

RETURNS
	uint16 bitmask.
*/
uint16 AghfpGetSupportedQceCodecModeIdsInHf(AGHFP *aghfp)
{
    return (aghfp->hf_codec_modes) ? aghfp->hf_codec_modes : CODEC_MODE_ID_UNSUPPORTED;
}

/****************************************************************************
NAME	
        AghfpGetSupportedQceCodecModeIdsInHF

DESCRIPTION
        If an SLC has been established and the HF and AG both support QCE mode,
        the bitmap of the HF mode IDs is put into mode_ids pointer, otherwise
        sets the value to CODEC_MODE_ID_UNSUPPORTED.

RETURNS
	codec_mode_id_type (see library.h) or #CODEC_MODE_ID_UNSUPPORTED.
*/
bool AghfpGetQceSelectedCodecModeId(AGHFP *aghfp, uint16 *mode_id)
{
    *mode_id = aghfp->qce_codec_mode_id;
    if (*mode_id == 0xFFFF){
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
