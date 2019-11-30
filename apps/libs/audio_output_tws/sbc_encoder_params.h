/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sbc_encoder_params.h
 
DESCRIPTION
    Converts sink application's format of SBC encoder settings
    to operators library SBC encoder parameters.
*/

#ifndef SBC_ENCODER_PARAMS_H_
#define SBC_ENCODER_PARAMS_H_

#include <audio_sbc_encoder_params.h>

/****************************************************************************
DESCRIPTION
    Converts sink application's format of SBC encoder settings
    to operators library SBC encoder parameters.
*/
sbc_encoder_params_t ConvertSbcEncoderParams(unsigned bitpool, unsigned settings);

#endif /* SBC_ENCODER_PARAMS_H_ */
