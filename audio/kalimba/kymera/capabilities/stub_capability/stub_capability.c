/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stub_capability.c
 * \ingroup  capabilities
 *
 *  A Stub implementation of a Capability that can be built and communicated
 *  with. This is provided to accelerate the development of new capabilities.
 *
 */

#include "capabilities.h"

/****************************************************************************
Private Function Definitions
*/
static void stub_capability_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/****************************************************************************
Private Constant Declarations
*/
#define STUB_CAPABILITY_ID  0xFFFF /* CHANGE THIS VALUE TO THAT SELECTED */

/** The stub capability function handler table */
const handler_lookup_struct stub_capability_handler_table =
{
    base_op_create,           /* OPCMD_CREATE */
    base_op_destroy,          /* OPCMD_DESTROY */
    base_op_start,            /* OPCMD_START */
    base_op_stop,             /* OPCMD_STOP */
    base_op_reset,            /* OPCMD_RESET */
    base_op_connect,          /* OPCMD_CONNECT */
    base_op_disconnect,       /* OPCMD_DISCONNECT */
    base_op_buffer_details,   /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,  /* OPCMD_DATA_FORMAT */
    base_op_get_sched_info    /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table - this is the set of operator
 * messages that the capability understands and will attempt to service. */
const opmsg_handler_lookup_table_entry stub_capability_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {0, NULL}
};


/* Capability data - This is the definition of the capability that Opmgr uses to
 * create the capability from. */
const CAPABILITY_DATA stub_capability_cap_data =
{
    STUB_CAPABILITY_ID,             /* Capability ID */
    0, 1,                           /* Version information - hi and lo parts */
    1, 1,                           /* Max number of sinks/inputs and sources/outputs */
    &stub_capability_handler_table, /* Pointer to message handler function table */
    stub_capability_opmsg_handler_table,    /* Pointer to operator message handler function table */
    stub_capability_process_data,           /* Pointer to data processing function */
    0,                              /* TODO: this would hold processing time information */
    0                               /* Size of capability-specific per-instance data */
};

/* Data processing function */
static void stub_capability_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    /*
     * Capability processing code goes here
     */
}

