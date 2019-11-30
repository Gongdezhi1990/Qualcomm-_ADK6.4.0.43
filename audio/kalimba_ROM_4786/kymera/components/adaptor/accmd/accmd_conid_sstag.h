/****************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#ifndef ACCMD_CONID_SSTAG_H
#define ACCMD_CONID_SSTAG_H

#include "subserv/subserv.h"

/**
 * \brief gets the connection database index of the connection corresponding to the
 * given service tag.
 *
 * \param stag Service Tag
 * \param pconidx pointer where connection index is to be returned. The returned
 * value is invalid if the function returns FALSE.
 *
 * \return returns FALSE if connection does not exist.
 *
 * This function will be normally be used by the accmd service and the AOV client.
 */
extern bool accmd_get_conidx_from_stag(SUBSERV_TAG stag, unsigned int *pconidx);

#endif // ACCMD_CONID_SSTAG_H
