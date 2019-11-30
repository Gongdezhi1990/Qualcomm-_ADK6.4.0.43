/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef STREAM_FOR_AUDIO_DATA_SERVICE_H
#define STREAM_FOR_AUDIO_DATA_SERVICE_H
#ifdef INSTALL_AUDIO_DATA_SERVICE

#include "audio_data_service.h"

#ifdef INSTALL_A2DP
/*
 * Functions from stream_a2dp_hydra.c
 * ----------------------------------
 */
/*
 * \brief queries a2dp ep for endpoint parameters
 *        that include data/meta buffer sizes, data buffer flags
 *        and kicks
 * \param params pointer to a endpoint parameter structure
 * \return TRUE if the query was successful, FALSE otherwise
 */
bool stream_query_a2dp(AUDIO_DATA_SERVICE_EP_PARAMS *params);

/*
 * \brief creates a2dp ep.
 *        Called from Audio data service.
 * \param cfg pointer to a endpoint configuration structure
 * \return create_res
 */
void stream_create_a2dp(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res);

/*
 * \brief destroys a2dp ep.
 *        Called from Audio data service.
 * \param ep pointer to a a2dp ep
 * \return TRUE if the destroy succeeded, FALSE otherwise
 */
bool stream_destroy_a2dp(void *ep);

#endif /* INSTALL_A2DP */

#ifdef INSTALL_USB_AUDIO
/*
 * Functions from stream_usb_audio_hydra.c
 * ----------------------------------
 */
/*
 * \brief queries usb_audio ep for endpoint parameters
 *        that include data/meta buffer sizes, data buffer flags
 *        and kicks
 * \param params pointer to a endpoint parameter structure
 * \return TRUE if the query was successful, FALSE otherwise
 */
bool stream_query_usb_audio(AUDIO_DATA_SERVICE_EP_PARAMS *params);

/*
 * \brief creates usb_audio ep.
 *        Called from Audio data service.
 * \param cfg pointer to a endpoint configuration structure
 * \return create_res
 */
void stream_create_usb_audio(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res);

/*
 * \brief destroys usb_audio ep.
 *        Called from Audio data service.
 * \param ep pointer to a usb_audio ep
 * \return TRUE if the destroy succeeded, FALSE otherwise
 */
bool stream_destroy_usb_audio(void *ep);

#endif /* INSTALL_USB_AUDIO */

#ifdef INSTALL_AUDIO_DATA_SERVICE_TESTER
/*
 * Functions from stream_tester_hydra.c
 * ------------------------------------
 */
/*
 * \brief queries tester ep for endpoint parameters
 *        that include data/meta buffer sizes, data buffer flags
 *        and kicks
 * \param params pointer to a endpoint parameter structure
 * \return TRUE if the query was successful, FALSE otherwise
 */
bool stream_query_tester(AUDIO_DATA_SERVICE_EP_PARAMS *params);

/*
 * \brief creates tester ep.
 *        Called from Audio data service.
 * \param cfg pointer to a endpoint configuration structure
 * \return create_res
 */
void stream_create_tester(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res);

/*
 * \brief destroys tester ep.
 *        Called from Audio data service.
 * \param ep pointer to a tester ep
 * \return TRUE if the destroy succeeded, FALSE otherwise
 */
bool stream_destroy_tester(void *ep);

#endif /* INSTALL_AUDIO_DATA_SERVICE_TESTER */

#ifdef INSTALL_FILE
/*
 * Functions from stream_file_hydra.c
 * ------------------------------------
 */
/*
 * \brief queries file ep for endpoint parameters
 *        that include data/meta buffer sizes, data buffer flags
 *        and kicks
 * \param params pointer to a endpoint parameter structure
 * \return TRUE if the query was successful, FALSE otherwise
 */
bool stream_query_file(AUDIO_DATA_SERVICE_EP_PARAMS *params);

/*
 * \brief creates file ep.
 *        Called from Audio data service.
 * \param cfg pointer to a endpoint configuration structure
 * \return create_res
 */
void stream_create_file(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res);

/*
 * \brief destroys file ep.
 *        Called from Audio data service.
 * \param ep pointer to a file ep
 * \return TRUE if the destroy succeeded, FALSE otherwise
 */
bool stream_destroy_file(void *ep);

#endif /* INSTALL_FILE */

#ifdef INSTALL_TIMESTAMPED_ENDPOINT
/*
 * Functions from stream_timestamped_hydra.c
 * ------------------------------------
 */
/*
 * \brief queries timestamped ep for endpoint parameters
 *        that include data/meta buffer sizes, data buffer flags
 *        and kicks
 * \param params pointer to a endpoint parameter structure
 * \return TRUE if the query was successful, FALSE otherwise
 */
bool stream_query_timestamped(AUDIO_DATA_SERVICE_EP_PARAMS *params);

/*
 * \brief creates timestamped ep.
 *        Called from Audio data service.
 * \param cfg pointer to a endpoint configuration structure
 * \return create_res
 */
void stream_create_timestamped(const AUDIO_DATA_SERVICE_EP_CFG *cfg, AUDIO_DATA_SERVICE_EP_CREATE_RESULT* create_res);

/*
 * \brief destroys timestamped ep.
 *        Called from Audio data service.
 * \param ep pointer to a timestamped ep
 * \return TRUE if the destroy succeeded, FALSE otherwise
 */
bool stream_destroy_timestamped(void *ep);

#endif /* INSTALL_FILE */


#endif /* INSTALL_AUDIO_DATA_SERVICE */
#endif /* STREAM_FOR_AUDIO_DATA_SERVICE_H */
