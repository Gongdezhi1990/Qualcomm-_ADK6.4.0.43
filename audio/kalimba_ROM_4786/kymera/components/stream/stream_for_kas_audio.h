/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef STREAM_FOR_KAS_AUDIO_H
#define STREAM_FOR_KAS_AUDIO_H

/**
 * \brief Gets the endpoint set as kick_object.
 *
 * \param *port pointer to the port structure that holds the 
 *              kick_object. 
 * \return pointer to the endpoint used as kick_object
 */
ENDPOINT* get_endpoint_to_kick(a7da_audio_port* port);

/**
 * \brief Sets the endpoint given endpoint as a kick_object.
 *
 * \param *port pointer to the port structure that holds the 
 *              kick_object.
 */
void set_endpoint_to_kick(a7da_audio_port* port, ENDPOINT* ep);

/**
 * \brief Kicks the master endpoint associated to the device
 *
 * \param *port pointer to the port structure to get the master 
 *         endpoint to be kicked
 */
void kick_master_endpoint(a7da_audio_port* port);

#endif /* STREAM_FOR_KAS_AUDIO_H */
