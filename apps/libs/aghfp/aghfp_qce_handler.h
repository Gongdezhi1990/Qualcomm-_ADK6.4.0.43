/****************************************************************************
Copyright (c) 2018 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    aghfp_qce_handler.h

DESCRIPTION
    Functions for handling Qualcommm Codec Extensions AT commands and state.
*/

#ifndef AGHFP_QCE_HANDLER_H_
#define AGHFP_QCE_HANDLER_H_

#include "aghfp_private.h"

/* The only packet_type supported for QCE Mode eSCO connections is 2-EV3. */
#define QCE_MODE_PACKET_TYPE (sync_3ev5 | sync_2ev5 | sync_3ev3)

void aghfpHandleHfCodecModes(AGHFP *aghfp, AGHFP_INTERNAL_AT_QAC_REQ_T *codec_modes);
void aghfpQceSendCodecSelection(AGHFP *aghfp, codec_mode_bit_type codec_mode_bit);
const aghfp_audio_params *aghfpGetQceEscoParams(codec_mode_bit_type codec_mode_bit);
void aghfpHandleHfCodecSelect(AGHFP *aghfp, AGHFP_INTERNAL_AT_QCS_REQ_T *codec_select);

#endif /* AGHFP_QCE_HANDLER_H_ */
