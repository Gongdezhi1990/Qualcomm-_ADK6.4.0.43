/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_usb_chain_config.h

DESCRIPTION
    USB audio input chain configuration.
*/

#ifndef AUDIO_INPUT_USB_CHAIN_CONFIG_H_
#define AUDIO_INPUT_USB_CHAIN_CONFIG_H_

#include <chain.h>

enum
{
    usb_rx_role,
	switched_passthrough_role
};

typedef enum
{
    path_encoded,
    path_pcm_left,
    path_pcm_right,
	path_pcm_forwarding
} path_channel_t;

/****************************************************************************
DESCRIPTION
    Return true if single channel rendering mode is enabled.
*/
bool isMonoMode(void);

/****************************************************************************
DESCRIPTION
    Return the chain configuration.
*/
const chain_config_t* AudioInputUsbGetChainConfig(void);


#endif /* AUDIO_INPUT_USB_CHAIN_CONFIG_H_ */
