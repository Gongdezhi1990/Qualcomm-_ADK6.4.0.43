/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup download_self_test
 * \file  download_self_test_private.h
 * \ingroup capabilities
 *
 * Download self-test operator private header file. <br>
 *
 */

#ifndef _DOWNLOAD_SELF_TEST_PRIVATE_H_
#define _DOWNLOAD_SELF_TEST_PRIVATE_H_
/*****************************************************************************
Include Files
*/
#include "capabilities.h"

/****************************************************************************
Public Constant Definitions
*/
#define DOWNLOAD_SELF_TEST_VERSION_LENGTH               2

/** default buffer size for this operator */
#define DOWNLOAD_SELF_TEST_DEFAULT_BUFFER_SIZE                  128
/** default block size for this operator's terminals */
#define DOWNLOAD_SELF_TEST_DEFAULT_BLOCK_SIZE                   1

/****************************************************************************
Public Type Declarations
*/

/* capability-specific extra operator data */
typedef struct
{
    /** The buffer at the input terminal */
    tCbuffer *ip_buffer;

    /** The buffer at the output terminal */
    tCbuffer *op_buffer;

    /** The audio data format configurations of the input terminal */
    AUDIO_DATA_FORMAT ip_format;

    /** The audio data format configurations of the output terminal */
    AUDIO_DATA_FORMAT op_format;

    /** working parameter used for various things */
    unsigned working_data;

} DOWNLOAD_SELF_TEST_OP_DATA;

/*****************************************************************************
Private Macro Declarations
*/
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

/*****************************************************************************
Private Function Definitions
*/
/* ASM functions */
extern unsigned int download_self_test_branch_test(void);
extern unsigned int download_self_test_var_ref_test_minim(void);
extern unsigned int download_self_test_var_ref_test_maxim(void);
extern unsigned int download_self_test_call_from_maxim_to_minim_test_maxim(void);
extern unsigned int download_self_test_variable_size_vars_test_maxim_asm(void);
extern unsigned int download_self_test_variable_size_vars_test_minim_asm(void);

/* C helper functions */
extern unsigned int download_self_test_util_helper(void);
extern unsigned int func_increase_one_with_param_c(unsigned int a);
extern void func_decrease_one(void);
extern void set_a(int n);


#endif /* _BASIC_MONO_PRIVATE_H_ */
