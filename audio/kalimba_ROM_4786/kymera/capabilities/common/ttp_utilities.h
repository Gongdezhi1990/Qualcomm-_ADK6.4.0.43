/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef TTP_UTILITIES_H
#define TTP_UTILITIES_H

#include "ttp/ttp.h"
#include "buffer/buffer_metadata.h"

/************************* Public constants and macros ******************* */



/***************************** Public functions *************************** */

/**
 * \brief Populate metadata tag with time-to-play information
 *
 * \param tag The tag to populate
 * \param status Status info from TTP
 */
extern void ttp_utils_populate_tag(metadata_tag *tag, ttp_status *status);

#endif /* TTP_UTILITIES_H */
