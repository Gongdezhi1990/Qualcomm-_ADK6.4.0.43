/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup sco Sco endpoint
 * \ingroup endpoints
 * \file stream_endpoint_sco.h
 *
 */

#ifndef _STREAM_ENDPOINT_SCO_H_
#define _STREAM_ENDPOINT_SCO_H_

/****************************************************************************
Include Files
*/
#include "stream/stream.h"


/****************************************************************************
Private Type Declarations
*/
/**
 * \brief get the audio data format of the underlying hardware associated with
 * the endpoint
 *
 * \param endpoint pointer to the endpoint to get the data format of.
 *
 * \return the data format of the underlying hardware
 */
AUDIO_DATA_FORMAT sco_get_data_format (ENDPOINT *endpoint);

/**
 * \brief set the audio data format that the endpoint will place in/consume from
 * the buffer
 *
 * \param endpoint pointer to the endpoint to set the data format of.
 * \param format AUDIO_DATA_FORMAT requested to be produced/consumed by the endpoint
 *
 * \return whether the set operation was successful
 */
bool sco_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format);
#ifdef INSTALL_SCO_RX_TOA_METADATA_GENERATE
#if defined(INSTALL_TTP) && defined(INSTALL_METADATA)
#define SCO_RX_GENERATE_METADATA
#endif /* defined(INSTALL_TTP) && defined(INSTALL_METADATA) */
#endif /* INSTALL_SCO_RX_TOA_METADATA_GENERATE */
#endif /* !_STREAM_ENDPOINT_SCO_H_ */
