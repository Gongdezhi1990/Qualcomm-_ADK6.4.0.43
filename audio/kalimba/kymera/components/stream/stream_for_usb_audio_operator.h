/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef STREAM_FOR_USB_AUDIO_ENDPOINT_H
#define STREAM_FOR_USB_AUDIO_ENDPOINT_H

/**
 * \brief configures the usb audio endpoint
 *
 * \param endpoint pointer to the endpoint that we want to configure
 * \param sample_rate sample rate of the endpoint
 * \param nrof_channels number of channels in the usb audio connection
 * \param subframe_size size of subframe in number of bits
 * \return TRUE if configured successfully else FALSE
 */
extern bool stream_usb_audio_configure_ep(ENDPOINT *ep, unsigned sample_rate, unsigned nrof_channels, unsigned subframe_size);

/**
 * stream_usb_audio_can_enact_rate_adjust
 * \brief querying the endpoint whether it can enact rate adjustment
 *
 * \param endpoint pointer to the endpoint that we want to query
 * \param rate_adjust_val if endpoint enacting the address of rate
 *        adjust value will be returned on this parameter, else NULL will
 *        be returned.
 * \return TRUE if query was successful else FALS;
 */
extern bool stream_usb_audio_can_enact_rate_adjust(ENDPOINT *ep, unsigned **target_rate_adjust_val);

/**
 * \brief sets the cbops op that is used for rate adjustment
 *
 * \param endpoint pointer to the usb audio endpoint
 * \param rate_adjust_op pointer to the rate adjustment op
 */
extern void stream_usb_audio_set_cbops_sra_op(ENDPOINT *ep, cbops_op* rate_adjust_op);

#endif /* STREAM_FOR_USB_AUDIO_ENDPOINT_H */
