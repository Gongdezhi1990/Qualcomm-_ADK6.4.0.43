/*****************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
  @file gaia_voice_assistant.h
  @brief Implementation of Voice Assistant functionality over GAIA.
*/

#ifndef _GAIA_VOICE_ASSISTANT_H_
#define _GAIA_VOICE_ASSISTANT_H_

#include "gaia_private.h"

/*! @brief Function to check that the host supports the device's
           version of the Voice Assistant protocol.
 *
 *  @param transport The gaia transport to conduct the check.
 */
void gaiaVoiceAssistantCheckHostVersion(gaia_transport *transport);

/*! @brief Function to handle a VA command received from the GAIA Host
 *
 *  @param transport The gaia transport confirmed.
 *  @param command_id The parsed command identifier
 *  @param payload_length The number of octets in the payload
 *  @param payload Pointer to the command payload
 */
bool gaiaHandleVoiceAssistantCommand(gaia_transport *transport, uint16 command_id,
                                      uint8 payload_length, uint8 *payload);

#endif /* _GAIA_VOICE_ASSISTANT_H_ */
