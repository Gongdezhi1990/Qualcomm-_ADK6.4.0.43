/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup exp_const Exported Constants
 * \ingroup mem_utils
 *
 * \file  
 * \ingroup exp_const
 *
 */
#ifndef EXPORTED_CONSTANTS_PRIVATE_H
#define EXPORTED_CONSTANTS_PRIVATE_H

#include "pmalloc/pl_malloc.h"      // Single file to avoid empty-compilation units

#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT

#include "platform/pl_interrupt.h"
#include "platform/pl_assert.h"

#include "fileserv/fileserv.h"

#include "gen/k32/system_prim.h"
#include "gen/k32/isp_prim.h"
#include "gen/k32/ftp_prim.h"
#include "gen/k32/filename_prim.h"
#include "gen/k32/kip_msg_prim.h"
#include "mem_utils/dynloader.h"
#include "sched_oxygen/sched_oxygen.h"
#include "kip_mgr/kip_mgr.h"
#include "kip/kip_msg_adaptor.h"
#include "adaptor/adaptor.h"

#include "hydra_log/hydra_log.h"

#include "hal/hal.h"
#include "hal/hal_dm_sections.h"

#include "exported_constants.h"
#include "patch/patch.h"

/** 
 * Message IDs for messages sent to MEM_UTILS_QUEUE
 *
 * The message format is defined local to exported_constants.c
 */
typedef enum {
        /** Add a user of specific constant. 
            Can initiate a file transfer */
    EXP_CONST_MSG_ADDREF_ID,
        /** Remove a user of a constant.
            Can lead to copy in local RAM being removed */
    EXP_CONST_MSG_RELEASE_ID,
        /** Queue a request for a callback message.
            The message is sent when it gets to the front of the queue,
            that is preceding messages have been processed */
    EXP_CONST_MSG_CALLBACK_ID,
        /** Message sent when a file transfer has ended.
            No content */
    EXP_CONST_MSG_TRANSFER_OVER_KICK_ID,
        /** Message sent to ourselves to process next item in queue 
            No content. */
    EXP_CONST_MSG_KICK_ID
} exp_const_msgid;

/** 
 * Status value of a constant.
 *
 * The states could be reduced to 'dont know', 'in ram', 'done with this' and
 * 'transient error' - but the extra states provide useful information for debug
 * purposes.
 */
typedef enum 
{
        /** Initial state, no information */
    EXP_CONST_STATUS_UNKNOWN,
        /** We know that the needed file exists */
    EXP_CONST_STATUS_EXISTS,
        /** File is being loaded (information status only) */
    EXP_CONST_STATUS_LOADING,
        /** File has been loaded, and is in RAM */
    EXP_CONST_STATUS_IN_RAM,
        /** File is in RAM, but we plan to delete it */
    EXP_CONST_STATUS_DELETABLE,


        /* All codes >= EXP_CONST_ERROR are errors */

        /** Error retrieving the file. Permanently fatal */
    EXP_CONST_STATUS_ERROR,
        /** Error retrieving the file due to lack of memory. 
         *  Subsequent attempts may be successful.
         */
    EXP_CONST_STATUS_ERROR_MEMORY,
        /** Error retrieving the file due to a session problem. 
         *  May succeed, although unlikely, subsequently.
         */
    EXP_CONST_STATUS_ERROR_SESSION

        /* ----> Only add error codes here <---- */
} exp_const_status;

/** Entry used to define mapping of a constant
 *
 * This is used to form a linked list of exported constants. This 
 * is never released as the basic information is of use.
 */
typedef struct exp_const_mapping_s
{
    void                       *const_in_ROM;
    exp_const_status            status;
    void                       *ram_copy;
    unsigned                   *uses;
    unsigned                    num_uses;
} exp_const_mapping;


/* exported_constants_file_xfer.c */
extern void exp_const_file_transferer_init(void);
extern void exp_const_load_external_table(exp_const_mapping *map);

#endif /* INSTALL_CAPABILITY_CONSTANT_EXPORT */

#endif /* EXPORTED_CONSTANTS_PRIVATE_H */
