/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  download_self_test.c
 * \ingroup  capabilities
 *
 *  Basic download self test capability
 *
 */
/****************************************************************************
Include Files
*/
#include "download_self_test_private.h"

/****************************************************************************
Private Function Definitions
*/
/* Message handlers */
static bool download_self_test_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
static bool download_self_test_opmsg_start_self_test(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Data processing function */
static void download_self_test_process_data_c(OPERATOR_DATA *, TOUCHED_TERMINALS*);
/*****************************************************************************
Private Constant Declarations
*/
/** The download self-test capability function handler table */
const handler_lookup_struct download_self_test_handler_table =
{
    download_self_test_create,  /* OPCMD_CREATE */
    base_op_destroy,            /* OPCMD_DESTROY */
    NULL,                       /* OPCMD_START */
    NULL,                       /* OPCMD_STOP */
    NULL,                       /* OPCMD_RESET */
    NULL,                       /* OPCMD_CONNECT */
    NULL,                       /* OPCMD_DISCONNECT */
    NULL,                       /* OPCMD_BUFFER_DETAILS */
    NULL,                       /* OPCMD_DATA_FORMAT */
    NULL                        /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry download_self_test_opmsg_handler_table[] =
    {{OPMSG_DOWNLOAD_SELF_TEST_ID_START_SELF_TEST,     download_self_test_opmsg_start_self_test},
    {0, NULL}};

const CAPABILITY_DATA download_self_test_cap_data =
{
    CAP_ID_DOWNLOAD_SELF_TEST,             /* Capability ID */
    0, 1,                                  /* Version information - hi and lo parts */
    1, 1,                                  /* Max number of sinks/inputs and sources/outputs */
    &download_self_test_handler_table,     /* Pointer to message handler function table */
    download_self_test_opmsg_handler_table,/* Pointer to operator message handler function table */
    download_self_test_process_data_c,     /* Pointer to data processing function */
    0,                                     /* TODO - Processing time information */
    sizeof(DOWNLOAD_SELF_TEST_OP_DATA)     /* Size of capability-specific per-instance data */
};

/* ********************************** API functions ************************************* */

static bool download_self_test_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    DOWNLOAD_SELF_TEST_OP_DATA *self_test_data;
    int i=0;
    AUDIO_LOG_STRING(string1, "Hello");
    AUDIO_LOG_STRING(string2, "Hello there");

    /* Test debug logging*/
    while(i<10000)
    {
        L0_DBG_MSG( "debug log test: Test normal logging. \n");
        L0_DBG_MSG1("debug log test: i = %d \n", i);
        L0_DBG_MSG2("debug log test: %d %d \n", 1, 2);
        L0_DBG_MSG3("debug log test: %d %d %d \n", 1, 2, 3);
        L0_DBG_MSG4("debug log test: %d %d %d %d \n", 1, 2, 3, 4);
        L0_DBG_MSG5("debug log test: %d %d %d %d %d \n", 1, 2, 3, 4, 5);

        L0_DBG_MSG( "debug log test: Test log level. Log 4 should be missing\n");
        L4_DBG_MSG1("debug log test: %d \n", 1);

        L0_DBG_MSG( "debug log test: Test string prints! \n");
        L0_DBG_MSG1("debug log test: %d \n", string1);
        L0_DBG_MSG2("debug log test: %s %d \n", string1, 1);
        L0_DBG_MSG2("debug log test: %s %s \n", string1, string2);
        L0_DBG_MSG4("debug log test: %s %s \n", 0, string1, string2, 1);

        L0_DBG_MSG( "debug log test: From here comes some errors! \n");
        L0_DBG_MSG4("debug log test: %d %d \n", 1, 2, 3, 4);
        L0_DBG_MSG3("debug log test: %d %d %d %d %d \n", 1, 2, 3);
        L0_DBG_MSG( "debug log test: End of the errors! \n");
        
        i++;
    }

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }
    self_test_data = (DOWNLOAD_SELF_TEST_OP_DATA *)op_data->extra_op_data;
    self_test_data->ip_format = AUDIO_DATA_FORMAT_FIXP;
    self_test_data->op_format = AUDIO_DATA_FORMAT_FIXP;
    return TRUE;
}


/* ************************************* Data processing-related functions and wrappers **********************************/

static void download_self_test_process_data_c(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    /* Does nothing, it's not expected to be called as we don't run as a normal operator */
    return ;
}

volatile unsigned char firstByte = 0xC;
volatile unsigned firstWord = 0x87654321UL;
volatile uint16 firstHalf = 0xABCD;
volatile unsigned char secondByte = 0x2A;
volatile unsigned char thirdByte = 0x2B;
volatile unsigned char fourthByte = 0x2C;
volatile unsigned char fifthByte = 0x2D;
volatile unsigned char ByteArray[3] = {0x2E, 0x2F, 0x30};
volatile unsigned secondWord = 0x63;
volatile unsigned char sixthByte = 0x31;
volatile unsigned secondHalf[3] = {0xBEEF,0xDEAD, 0xABCD};
volatile uint16 secondHalf1 = 0xBEEF;

static unsigned int download_self_test_variable_size_vars_test_c(void)
{
    if (firstByte != 0xC)
    {
        return 0xBAD;
    }
    if (firstWord != 0x87654321UL)
    {
        return 0xBAD;
    }
    if (firstHalf != 0xABCD)
    {
        return 0xBAD;
    }
    if (secondByte != 0x2A)
    {
        return 0xBAD;
    }
    if (thirdByte != 0x2B)
    {
        return 0xBAD;
    }
    if (fourthByte != 0x2C)
    {
        return 0xBAD;
    }
    if (fifthByte != 0x2D)
    {
        return 0xBAD;
    }
    if (ByteArray[2] != 0x30)
    {
        return 0xBAD;
    }
    if (secondWord != 0x63)
    {
        return 0xBAD;
    }
    if (sixthByte != 0x31)
    {
        return 0xBAD;
    }
    if (secondHalf[1] != 0xDEAD)
    {
        return 0xBAD;
    }
    if (secondHalf1 != 0xBEEF)
    {
        return 0xBAD;
    }
    return 0x600D;
}

/* **************************** Operator message handlers ******************************** */

/* Duplicated in download_self_test_util.c */
static unsigned int my_static_array[] = {0, 1, 2};

static bool download_self_test_opmsg_start_self_test(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    unsigned int test_result = 0xBAD;
    /* Compiler will convert func_ptr into an anonymous symbol L_lcx, and pull in C-runtime __CRT_wcpy function
     * Build system must deal with this
     * */
    unsigned int (*func_ptr[])(void) = {&download_self_test_branch_test, &download_self_test_var_ref_test_minim, &download_self_test_var_ref_test_maxim};
    unsigned int* my_local_array[] = {&my_static_array[0], &my_static_array[1], &my_static_array[2]};
    unsigned int i;
    L4_DBG_MSG("download_self_test_opmsg_start_self_test  \n");
    set_a(1);
    /* Set up the a default success response information */
    /* length is 1 word, msgID */
    *resp_length = OPMSG_RSP_PAYLOAD_SIZE_RAW_DATA(0);
	*resp_data = (OP_OPMSG_RSP_PAYLOAD *)xpnewn(*resp_length, unsigned);
    if (*resp_data == NULL)
    {
        return FALSE;
    }
    /* Check calls between Minim/Maxim code sections */
    test_result = download_self_test_call_from_maxim_to_minim_test_maxim();
    if (test_result != 0x600D)
    {
        return FALSE;
    }
    /* Check multiple variable sizes referencing */
    test_result = download_self_test_variable_size_vars_test_maxim_asm();
    if (test_result != 0x600D)
    {
        return FALSE;
    }
    test_result = download_self_test_variable_size_vars_test_minim_asm();
    if (test_result != 0x600D)
    {
        return FALSE;
    }
    test_result = download_self_test_variable_size_vars_test_c();
    if (test_result != 0x600D)
    {
        return FALSE;
    }
    /* Call assembly functions and change the already allocated response to the result of the test */
    for (i = 0; i < ARRAY_SIZE(func_ptr); i++)
    {
        test_result = func_ptr[i]();
        if (test_result != 0x600D)
        {
            /* Return BAD (0xBAD) */
            (*resp_data)->msg_id = test_result;
            return TRUE;
        }
    }
    /* This loop forces the creation of anonymous initialisation tables L_lcx */
    for (i = 0; i < ARRAY_SIZE(my_static_array); i++)
    {
        my_static_array[i] = *my_local_array[i];
    }
    /* Force linking of another C module for this capability */
    my_static_array[0] = download_self_test_util_helper();
    /* return GOOD (0x600D) or BAD (0xBAD)*/
    (*resp_data)->msg_id = my_static_array[0];
    return TRUE;
}
