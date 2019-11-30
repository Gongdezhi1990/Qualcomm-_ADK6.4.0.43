/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_tws_chain.h

DESCRIPTION
    TWS chain functionality
*/

#ifndef AUDIO_OUTPUT_TWS_CHAIN_H_
#define AUDIO_OUTPUT_TWS_CHAIN_H_

void AudioOutputTwsChainCreate(const audio_plugin_forwarding_params_t* params);

void AudioOutputTwsChainStart(void);

void AudioOutputTwsChainStop(void);

void AudioOutputTwsChainDestroy(void);

Source AudioOutputTwsChainGetLeftSource(void);

Source AudioOutputTwsChainGetRightSource(void);

Source AudioOutputTwsChainGetForwardingSource(void);

#endif /* AUDIO_OUTPUT_TWS_CHAIN_H_ */
