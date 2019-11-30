/****************************************************************************
 * Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
 * \file  @@@cap_name@@@.c
 * \ingroup  capabilities
 *
 *  A Stub implementation of a Capability that can be built and communicated
 *  with. This is provided to accelerate the development of new capabilities.
 *
 */

#include "capabilities.h"
#include "@@@cap_name@@@.h"

/****************************************************************************
Private Function Definitions
*/
static void @@@cap_name@@@_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/****************************************************************************
Private Constant Declarations
*/
#define @@@cap_name^U@@@_ID  @@@cap_id@@@ /* CHANGE THIS VALUE TO THAT SELECTED */

/** The stub capability function handler table */
const handler_lookup_struct @@@cap_name@@@_handler_table =
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
const opmsg_handler_lookup_table_entry @@@cap_name@@@_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {0, NULL}
};


/* Capability data - This is the definition of the capability that Opmgr uses to
 * create the capability from. */
const CAPABILITY_DATA @@@cap_name@@@_cap_data =
{
    @@@cap_name^U@@@_ID,             /* Capability ID */
    0, 1,                           /* Version information - hi and lo parts */
    @@@max_inputs@@@, @@@max_outputs@@@,                           /* Max number of sinks/inputs and sources/outputs */
    &@@@cap_name@@@_handler_table, /* Pointer to message handler function table */
    @@@cap_name@@@_opmsg_handler_table,    /* Pointer to operator message handler function table */
    @@@cap_name@@@_process_data,           /* Pointer to data processing function */
    0,                              /* TODO: this would hold processing time information */
    sizeof(@@@cap_name^U@@@_OP_DATA)       /* Size of capability-specific per-instance data */
};
MAP_INSTANCE_DATA(@@@cap_name^U@@@_ID, @@@cap_name^U@@@_OP_DATA)

/* Accessing the capability-specific per-instance data function */
static inline @@@cap_name^U@@@_OP_DATA *get_instance_data(OPERATOR_DATA *op_data)
{
    return (@@@cap_name^U@@@_OP_DATA *) base_op_get_instance_data(op_data);
}

/* Data processing function */
static void @@@cap_name@@@_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    /*
     * TODO Capability data processing code goes here...
     * This code is provided for demonstration purposes only. It shows how you can call into your private Library
     * This example library simulates a key verification algorithm although all it does
     * is an XOR operation with an internal "secret" key so do not use this for any serious purpose
     */
    int my_key = 0xB70C5F1FUL;
    @@@cap_name^U@@@_OP_DATA *@@@cap_name@@@_data = get_instance_data(op_data);
    /* Call into your private library */
    if (@@@plib_name@@@_entry(my_key))
    {
        /*
         * Assembly code can be called for faster processing ... (see @@@cap_name@@@_proc.asm)
         */ 
        @@@cap_name@@@_proc_func(@@@cap_name@@@_data);
    }
}
