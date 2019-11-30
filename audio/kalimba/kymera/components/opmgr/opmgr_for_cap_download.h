/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup opmgr Operator Manager
 * \file  opmgr_for_cap_download.h
 * \ingroup opmgr
 *
 * Operator Manager header file used by capability download. <br>
 *
 */

#ifndef _OPMGR_FOR_CAPABILITY_DOWNLOAD_H_
#define _OPMGR_FOR_CAPABILITY_DOWNLOAD_H_

#include "opmgr_for_ops.h"

/****************************************************************************
Private Type Declarations
*/
typedef enum
{
    CAP_INSTALLED,
    CAP_DOWNLOADING,
    CAP_DOWNLOADED
} CAP_DOWNLOAD_STATUS;

typedef struct DOWNLOAD_CAP_DATA_DB{
    CAPABILITY_DATA *cap;
    CAP_DOWNLOAD_STATUS status;
    struct DOWNLOAD_CAP_DATA_DB *next;
} DOWNLOAD_CAP_DATA_DB;

/****************************************************************************
Public Function Declarations
*/

/**
 * \brief  Add capability to the list of downloadable capabilities.
 *
 * \param  cap_info - Pointer to a structure of information about capability. Note
 *         that function and handlers pointers will be 0 as they will be fixed up 
 *         once the download has been processed
 *
 * \return  True if capability got installed correctly. False otherwise.
 */
extern bool opmgr_install_capability(CAPABILITY_DATA* cap_info);

/**
 * \brief  Remove capability from the list of downloadable capabilities.
 *
 * \param  cap_id - Identifier of the capability  to remove
 *
 * \return  True if capability got installed correctly. False otherwise.
 */
extern bool opmgr_uninstall_capability(unsigned cap_id);

/**
 * \brief Get the state of a download capability
 *
 * \param  cap_id The capability id to check
 *
 * \param  status Used to return the state value
 *
 * \return  True if capability exists and is downloadable, False otherwise
 */
extern bool opmgr_get_download_cap_status(unsigned cap_id, CAP_DOWNLOAD_STATUS *status);

/**
 * \brief Set a download capability state
 *
 * \param  cap_id The capability id to change the state
 *
 * \param  status If capability exists, status changes to one of CAP_DOWNLOAD_STATUS
 *
 * \return  True if capability exists and is downloadable, False otherwise
 */
extern bool opmgr_set_download_cap_status(unsigned cap_id, CAP_DOWNLOAD_STATUS status);

#endif
