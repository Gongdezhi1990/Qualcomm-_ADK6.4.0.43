/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef STREAM_FOR_HAL_AUDIO_SPDIF_H
#define STREAM_FOR_HAL_AUDIO_SPDIF_H

#ifdef CHIP_BASE_HYDRA
/** Enumeration of keys for handling SPDIF events detected by hal */
typedef enum
{
   /* input rate changed */
   SPDIF_EVENT_RATE_CHANGE = 0,

   /* a change in channel status detected */
   SPDIF_EVENT_CHSTS_CHANGE = 1,

   /* block start event */
   SPDIF_EVENT_BLOCK_START = 2

} SPDIF_RX_EVENT;

/**
 * hydra_spdif_ep_event_handler
 *
 * \brief handle events raised by spdif hal
 *
 * \param event determines type of event
 * \param instance the spdif instance that event happened on
 * \param channel channel number within the instance
 * \param message_length length of the message
 * \param message payload of the message
 *
 * \return the created end point, NULL will be returned if the endpoint cannot be created.
 */

void hydra_spdif_ep_event_handler(SPDIF_RX_EVENT event,
                                  unsigned instance,
                                  unsigned channel,
                                  unsigned message_length,
                                  const void *message);

#endif /* CHIP_BASE_HYDRA */
#endif /* STREAM_FOR_HAL_AUDIO_SPDIF_H */
